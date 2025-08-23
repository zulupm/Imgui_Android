#include "websocket_client.h"
#include "logger.h"

WebSocketClient::WebSocketClient()
    : context_(nullptr), wsi_(nullptr), state_(WSState::DISCONNECTED), running_(false) {}

WebSocketClient::~WebSocketClient() {
    disconnect();
}

void WebSocketClient::connect(const std::string &url) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ == WSState::CONNECTING || state_ == WSState::CONNECTED)
        return;
    url_ = url;
    state_ = WSState::CONNECTING;
    running_ = true;
    thread_ = std::thread(&WebSocketClient::run, this);
}

void WebSocketClient::disconnect() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    if (thread_.joinable())
        thread_.join();
    if (context_) {
        lws_context_destroy(context_);
        context_ = nullptr;
    }
    wsi_ = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        state_ = WSState::DISCONNECTED;
    }
}

WSState WebSocketClient::state() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

std::vector<std::string> WebSocketClient::logs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return logs_;
}

int WebSocketClient::callback(struct lws *wsi, enum lws_callback_reasons reason,
                              void *user, void *in, size_t len) {
    WebSocketClient *self = reinterpret_cast<WebSocketClient *>(lws_context_user(lws_get_context(wsi)));
    switch (reason) {
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            if (self) {
                std::lock_guard<std::mutex> lock(self->mutex_);
                self->logs_.push_back("Connection error");
                self->state_ = WSState::ERROR;
            }
            break;
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            if (self) {
                std::lock_guard<std::mutex> lock(self->mutex_);
                self->logs_.push_back("Connected");
                self->state_ = WSState::CONNECTED;
            }
            break;
        case LWS_CALLBACK_CLOSED:
            if (self) {
                std::lock_guard<std::mutex> lock(self->mutex_);
                self->logs_.push_back("Disconnected");
                self->state_ = WSState::DISCONNECTED;
            }
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:
            if (self) {
                std::string msg(static_cast<const char *>(in), len);
                std::lock_guard<std::mutex> lock(self->mutex_);
                self->logs_.push_back("Recv: " + msg);
            }
            break;
        default:
            break;
    }
    return 0;
}

void WebSocketClient::run() {
    lws_protocols protocols[] = {
        {"ws", WebSocketClient::callback, 0, 0},
        {nullptr, nullptr, 0, 0}
    };

    lws_context_creation_info info = {};
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.user = this;
    info.fd_limit_per_thread = 1 + 1;

    context_ = lws_create_context(&info);
    if (!context_) {
        std::lock_guard<std::mutex> lock(mutex_);
        logs_.push_back("Failed to create context");
        state_ = WSState::ERROR;
        running_ = false;
        return;
    }

    std::string address = url_;
    std::string path = "/";
    auto pos = address.find('/');
    if (pos != std::string::npos) {
        path = address.substr(pos);
        address = address.substr(0, pos);
    }

    lws_client_connect_info ccinfo = {};
    ccinfo.context = context_;
    ccinfo.address = address.c_str();
    ccinfo.port = 443;
    ccinfo.path = path.c_str();
    ccinfo.host = address.c_str();
    ccinfo.origin = "origin";
    ccinfo.protocol = protocols[0].name;
    ccinfo.ssl_connection = LCCSCF_USE_SSL;

    wsi_ = lws_client_connect_via_info(&ccinfo);
    if (!wsi_) {
        std::lock_guard<std::mutex> lock(mutex_);
        logs_.push_back("Failed to initiate connection");
        state_ = WSState::ERROR;
        running_ = false;
        return;
    }

    while (running_) {
        lws_service(context_, 100);
    }
}

