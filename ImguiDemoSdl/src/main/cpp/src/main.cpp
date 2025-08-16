#include <SDL.h>
#include "imgui.h"

#include "logger.h"

#ifdef __ANDROID__
#include <GLES2/gl2.h>
#else
#include "gl_glcore_3_3.h"
#endif
#include "implot.h"

#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <algorithm>

#ifdef HAS_CURL
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#endif

/* Shader version definition for dear imgui */
const char* imguiShaderVersions = nullptr;

/**
 * A convenience function to create a context for the specified window
 * @param w Pointer to SDL_Window
 * @return An SDL_Context value
 */
static SDL_GLContext createCtx(SDL_Window *w)
{
    // Prepare and create context
#ifdef __ANDROID__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GLContext ctx = SDL_GL_CreateContext(w);

    if (!ctx)
    {
        Log(LOG_ERROR) << "Could not create context! SDL reports error: " << SDL_GetError();
        return ctx;
    }

    int major, minor, mask;
    int r, g, b, a, depth;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &mask);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

    SDL_GL_GetAttribute(SDL_GL_RED_SIZE,   &r);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &g);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE,  &b);
    SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &a);

    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);

    const char* mask_desc;

    if (mask & SDL_GL_CONTEXT_PROFILE_CORE) {
        mask_desc = "core";
    } else if (mask & SDL_GL_CONTEXT_PROFILE_COMPATIBILITY) {
        mask_desc = "compatibility";
    } else if (mask & SDL_GL_CONTEXT_PROFILE_ES) {
        mask_desc = "es";
    } else {
        mask_desc = "?";
    }

    Log(LOG_INFO) << "Got context: " << major << "." << minor << mask_desc
                  << ", R" << r << "G" << g << "B" << b << "A" << a << ", depth bits: " << depth;

    SDL_GL_MakeCurrent(w, ctx);
#ifdef __ANDROID__
    if (major == 3)
    {
        Log(LOG_INFO) << "Initializing ImGui for GLES3";
        imguiShaderVersions = "#version 300 es";
    }
    else
    {
        Log(LOG_INFO) << "Initializing ImGui for GLES2";
        imguiShaderVersions = "#version 100";
    }
#else
    imguiShaderVersions = "#version 330 core";
#endif
    Log(LOG_INFO) << "Finished initialization";
    return ctx;
}

#ifdef HAS_CURL
static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

static bool FetchPrices(const char* symbol, std::vector<double>& prices) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string response;
    std::string url = std::string("https://api.binance.com/api/v3/klines?symbol=") + symbol + "&interval=1h&limit=100";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) return false;

    try {
        auto data = nlohmann::json::parse(response);
        for (const auto& candle : data) {
            double price = std::stod(candle[4].get<std::string>());
            prices.push_back(price);
        }
    } catch (...) {
        return false;
    }
    return true;
}
#else
static bool FetchPrices(const char*, std::vector<double>&) {
    return false;
}
#endif


int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
#ifdef HAS_CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
#endif

    if (argc < 2)
    {
        Log(LOG_FATAL) << "Not enough arguments! Usage: " << argv[0] << " path_to_data_dir";
        SDL_Quit();
        return 1;
    }
    if (chdir(argv[1])) {
        Log(LOG_ERROR) << "Could not change directory properly!";
    } else {
        dirent **namelist;
        int numdirs = scandir(".", &namelist, NULL, alphasort);
        if (numdirs < 0) {
            Log(LOG_ERROR) << "Could not list directory";
        } else {
            for (int dirid = 0; dirid < numdirs; ++dirid) {
                Log(LOG_INFO) << "Got file: " << namelist[dirid]->d_name;
            }
            free(namelist);
        }
    }

    // Create window
    Log(LOG_INFO) << "Creating SDL_Window";
    SDL_Window *window = SDL_CreateWindow("Demo App", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext ctx = createCtx(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, ctx);
    ImGui_ImplOpenGL3_Init(imguiShaderVersions); // Select proper OpenGL version automagically

    ImGui::StyleColorsDark();

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
    io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 32.0f);

    bool show_test_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);

    std::vector<double> btc_prices;
    std::vector<double> eth_prices;
    FetchPrices("BTCUSDT", btc_prices);
    FetchPrices("ETHUSDT", eth_prices);
    size_t sample_count = std::min(btc_prices.size(), eth_prices.size());
    btc_prices.resize(sample_count);
    eth_prices.resize(sample_count);
    std::vector<double> sample_x(sample_count);
    for (size_t i = 0; i < sample_count; ++i) sample_x[i] = static_cast<double>(i);

    Log(LOG_INFO) << "Entering main loop";
    {
        bool done = false;
        while (!done) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                ImGui_ImplSDL2_ProcessEvent(&e);
                if (e.type == SDL_QUIT)
                    done = true;
            }
            if (io.WantTextInput) {
                SDL_StartTextInput();
            } else {
                SDL_StopTextInput();
            }
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            // 1. Show a simple window
            // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
            {
                static float f = 0.0f;
                ImGui::Text("Hello, world!");
                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
                ImGui::ColorEdit3("clear color", (float *) &clear_color);
                if (ImGui::Button("Test Window")) show_test_window ^= 1;
                if (ImGui::Button("Another Window")) show_another_window ^= 1;
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                            ImGui::GetIO().Framerate);
            }

            // 2. Show another simple window, this time using an explicit Begin/End pair
            if (show_another_window) {
                ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
                ImGui::Begin("Another Window", &show_another_window);
                ImGui::Text("Hello");
                ImGui::End();
            }

            // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowDemoWindow()
            if (show_test_window) {
                ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
                ImGui::ShowDemoWindow(&show_test_window);
            }

            // 4. Show crypto price plots
            if (ImGui::Begin("Crypto Prices")) {
                if (ImPlot::BeginPlot("BTC/ETH")) {
                    if (sample_count > 0) {
                        ImPlot::PlotLine("BTC", sample_x.data(), btc_prices.data(), (int)sample_count);
                        ImPlot::PlotLine("ETH", sample_x.data(), eth_prices.data(), (int)sample_count);
                    }
                    ImPlot::EndPlot();
                }
            }
            ImGui::End();

            // Rendering
            glViewport(0, 0, (int) ImGui::GetIO().DisplaySize.x, (int) ImGui::GetIO().DisplaySize.y);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(window);
        }
    }
    ImPlot::DestroyContext();
#ifdef HAS_CURL
    curl_global_cleanup();
#endif
    SDL_GL_DeleteContext(ctx);
    SDL_Quit();
    return 0;
}