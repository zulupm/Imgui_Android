#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>

#include <libwebsockets.h>

enum class WSState { DISCONNECTED, CONNECTING, CONNECTED, ERROR };

class WebSocketClient {
public:
    WebSocketClient();
    ~WebSocketClient();

    void connect(const std::string &url);
    void disconnect();

    WSState state() const;
    std::vector<std::string> logs() const;

private:
    static int callback(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len);
    void run();

    struct lws_context *context_;
    struct lws *wsi_;
    std::thread thread_;
    std::string url_;
    mutable std::mutex mutex_;
    WSState state_;
    std::vector<std::string> logs_;
    bool running_;
};

