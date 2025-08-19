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
#include "easywsclient.hpp"
#include <nlohmann/json.hpp>

using easywsclient::WebSocket;

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

static bool ToggleSwitch(const char* label, bool* v) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float height = ImGui::GetFrameHeight();
    float width = height * 1.6f;
    float radius = height * 0.5f;
    std::string id = std::string("##") + label;
    ImGui::InvisibleButton(id.c_str(), ImVec2(width, height));
    bool toggled = false;
    if (ImGui::IsItemClicked()) {
        *v = !*v;
        toggled = true;
    }
    ImU32 col_bg = *v ? ImGui::GetColorU32(ImGuiCol_Button) : ImGui::GetColorU32(ImGuiCol_FrameBg);
    if (ImGui::IsItemHovered())
        col_bg = *v ? ImGui::GetColorU32(ImGuiCol_ButtonHovered) : ImGui::GetColorU32(ImGuiCol_FrameBgHovered);
    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, radius);
    draw_list->AddCircleFilled(ImVec2(*v ? p.x + width - radius : p.x + radius, p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
    return toggled;
}

static void ScalePlotStyle(ImPlotStyle& style, float scale) {
    style.LineWeight *= scale;
    style.MarkerSize *= scale;
    style.MarkerWeight *= scale;
    style.ErrorBarSize *= scale;
    style.ErrorBarWeight *= scale;
    style.DigitalBitHeight *= scale;
    style.DigitalBitGap *= scale;
    style.PlotBorderSize *= scale;
    style.MajorTickLen = ImVec2(style.MajorTickLen.x * scale, style.MajorTickLen.y * scale);
    style.MinorTickLen = ImVec2(style.MinorTickLen.x * scale, style.MinorTickLen.y * scale);
    style.MajorTickSize = ImVec2(style.MajorTickSize.x * scale, style.MajorTickSize.y * scale);
    style.MinorTickSize = ImVec2(style.MinorTickSize.x * scale, style.MinorTickSize.y * scale);
    style.MajorGridSize = ImVec2(style.MajorGridSize.x * scale, style.MajorGridSize.y * scale);
    style.MinorGridSize = ImVec2(style.MinorGridSize.x * scale, style.MinorGridSize.y * scale);
    style.PlotPadding = ImVec2(style.PlotPadding.x * scale, style.PlotPadding.y * scale);
    style.LabelPadding = ImVec2(style.LabelPadding.x * scale, style.LabelPadding.y * scale);
    style.LegendPadding = ImVec2(style.LegendPadding.x * scale, style.LegendPadding.y * scale);
    style.LegendInnerPadding = ImVec2(style.LegendInnerPadding.x * scale, style.LegendInnerPadding.y * scale);
    style.LegendSpacing = ImVec2(style.LegendSpacing.x * scale, style.LegendSpacing.y * scale);
    style.MousePosPadding = ImVec2(style.MousePosPadding.x * scale, style.MousePosPadding.y * scale);
    style.AnnotationPadding = ImVec2(style.AnnotationPadding.x * scale, style.AnnotationPadding.y * scale);
    style.FitPadding = ImVec2(style.FitPadding.x * scale, style.FitPadding.y * scale);
    style.PlotDefaultSize = ImVec2(style.PlotDefaultSize.x * scale, style.PlotDefaultSize.y * scale);
    style.PlotMinSize = ImVec2(style.PlotMinSize.x * scale, style.PlotMinSize.y * scale);
}

static void ApplyMaterialTheme(bool dark, ImVec4& clear_color, float scale) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImVec4 accent = dark ? ImVec4(0.73f, 0.53f, 0.99f, 1.0f) : ImVec4(0.26f, 0.47f, 0.96f, 1.0f);
    ImVec4 bg = dark ? ImVec4(0.12f, 0.12f, 0.12f, 1.0f) : ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    ImVec4 surface = dark ? ImVec4(0.18f, 0.18f, 0.18f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 text = dark ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

    if (dark)
        ImGui::StyleColorsDark();
    else
        ImGui::StyleColorsLight();

    colors[ImGuiCol_Text]               = text;
    colors[ImGuiCol_WindowBg]           = bg;
    colors[ImGuiCol_ChildBg]            = bg;
    colors[ImGuiCol_PopupBg]            = surface;
    colors[ImGuiCol_FrameBg]            = surface;
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(accent.x, accent.y, accent.z, 0.4f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(accent.x, accent.y, accent.z, 0.6f);
    colors[ImGuiCol_Button]             = ImVec4(accent.x, accent.y, accent.z, 0.8f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(accent.x, accent.y, accent.z, 1.0f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(accent.x, accent.y, accent.z, 0.6f);
    colors[ImGuiCol_Header]             = ImVec4(accent.x, accent.y, accent.z, 0.8f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(accent.x, accent.y, accent.z, 1.0f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(accent.x, accent.y, accent.z, 0.6f);
    colors[ImGuiCol_Tab]                = surface;
    colors[ImGuiCol_TabHovered]         = colors[ImGuiCol_ButtonHovered];
    colors[ImGuiCol_TabActive]          = colors[ImGuiCol_ButtonActive];
    colors[ImGuiCol_TabUnfocused]       = surface;
    colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TabActive];
    colors[ImGuiCol_CheckMark]          = accent;
    colors[ImGuiCol_SliderGrab]         = accent;
    colors[ImGuiCol_SliderGrabActive]   = accent;

    style.WindowRounding = 6.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.FramePadding = ImVec2(12.0f, 8.0f);
    style.ItemSpacing = ImVec2(8.0f, 12.0f);
    style.ScrollbarSize = 24.0f;
    style.GrabMinSize = 20.0f;
    style.TouchExtraPadding = ImVec2(8.0f, 8.0f);
    style.ScaleAllSizes(scale);

    ImPlotStyle& plotStyle = ImPlot::GetStyle();
    ScalePlotStyle(plotStyle, scale);
    plotStyle.Colors[ImPlotCol_Line]          = accent;
    plotStyle.Colors[ImPlotCol_Fill]          = ImVec4(accent.x, accent.y, accent.z, 0.25f);
    plotStyle.Colors[ImPlotCol_MarkerOutline] = accent;
    plotStyle.Colors[ImPlotCol_MarkerFill]    = accent;
    plotStyle.Colors[ImPlotCol_FrameBg]       = surface;
    plotStyle.Colors[ImPlotCol_PlotBg]        = bg;
    plotStyle.Colors[ImPlotCol_PlotBorder]    = dark ? ImVec4(0.3f, 0.3f, 0.3f, 1.0f) : ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    plotStyle.Colors[ImPlotCol_LegendBg]      = surface;
    plotStyle.Colors[ImPlotCol_LegendBorder]  = plotStyle.Colors[ImPlotCol_PlotBorder];
    plotStyle.Colors[ImPlotCol_LegendText]    = text;
    plotStyle.Colors[ImPlotCol_TitleText]     = text;
    plotStyle.Colors[ImPlotCol_AxisText]      = text;
    plotStyle.Colors[ImPlotCol_AxisGrid]      = plotStyle.Colors[ImPlotCol_PlotBorder];
    plotStyle.Colors[ImPlotCol_AxisTick]      = plotStyle.Colors[ImPlotCol_PlotBorder];
    plotStyle.Colors[ImPlotCol_Selection]     = ImVec4(accent.x, accent.y, accent.z, 0.25f);
    plotStyle.Colors[ImPlotCol_Crosshairs]    = text;
    plotStyle.LineWeight = 2.0f * scale;
    plotStyle.MarkerSize = 6.0f * scale;
    plotStyle.MarkerWeight = 1.0f * scale;

    clear_color = bg;
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
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

    int display_w, display_h;
    SDL_GetWindowSize(window, &display_w, &display_h);
    float ui_scale = std::max(1.0f, static_cast<float>(display_w) / 1280.0f);

    ImVec4 clear_color;
    bool dark_mode = true;
    bool show_settings = false;
    ApplyMaterialTheme(dark_mode, clear_color, ui_scale);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
    io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 32.0f * ui_scale);

    bool show_test_window = true;
    bool show_another_window = false;

    std::vector<double> btc_x, btc_prices;
    std::vector<double> eth_x, eth_prices;
    WebSocket::pointer ws_btc = WebSocket::from_url("wss://stream.binance.com:9443/ws/btcusdt@trade");
    WebSocket::pointer ws_eth = WebSocket::from_url("wss://stream.binance.com:9443/ws/ethusdt@trade");
    const size_t max_points = 1000;

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
            if (ws_btc) {
                ws_btc->poll();
                ws_btc->dispatch([&btc_prices, &btc_x, max_points](const std::string& msg){
                    try {
                        auto j = nlohmann::json::parse(msg);
                        double price = std::stod(j["p"].get<std::string>());
                        btc_prices.push_back(price);
                        btc_x.push_back(static_cast<double>(btc_x.size()));
                        if (btc_prices.size() > max_points) {
                            btc_prices.erase(btc_prices.begin());
                            btc_x.erase(btc_x.begin());
                        }
                    } catch (...) {}
                });
            }
            if (ws_eth) {
                ws_eth->poll();
                ws_eth->dispatch([&eth_prices, &eth_x, max_points](const std::string& msg){
                    try {
                        auto j = nlohmann::json::parse(msg);
                        double price = std::stod(j["p"].get<std::string>());
                        eth_prices.push_back(price);
                        eth_x.push_back(static_cast<double>(eth_x.size()));
                        if (eth_prices.size() > max_points) {
                            eth_prices.erase(eth_prices.begin());
                            eth_x.erase(eth_x.begin());
                        }
                    } catch (...) {}
                });
            }
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            ImVec2 btn_size(36.0f * ui_scale, 36.0f * ui_scale);
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - btn_size.x - 8.0f, 8.0f), ImGuiCond_Always);
            ImGui::SetNextWindowSize(btn_size);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
            ImGui::Begin("SettingsButton", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
            if (ImGui::Button("⚙"))
                show_settings = !show_settings;
            ImGui::End();
            ImGui::PopStyleVar();

            if (show_settings) {
                float panel_width = 280.0f * ui_scale;
                ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - panel_width, 0), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(panel_width, io.DisplaySize.y));
                ImGui::Begin("Settings", &show_settings, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
                ImGui::TextUnformatted("Appearance");
                ImGui::Separator();
                if (ToggleSwitch("Dark mode", &dark_mode)) {
                    ApplyMaterialTheme(dark_mode, clear_color, ui_scale);
                }
                ImGui::End();
            }

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
                    if (!btc_prices.empty())
                        ImPlot::PlotLine("BTC", btc_x.data(), btc_prices.data(), (int)btc_prices.size());
                    if (!eth_prices.empty())
                        ImPlot::PlotLine("ETH", eth_x.data(), eth_prices.data(), (int)eth_prices.size());
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
    if (ws_btc) ws_btc->close();
    if (ws_eth) ws_eth->close();
    ImPlot::DestroyContext();
    SDL_GL_DeleteContext(ctx);
    SDL_Quit();
    return 0;
}