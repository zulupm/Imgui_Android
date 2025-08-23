# Cmake file for a sample cross-platform project with dear imgui.
# The aim here is to have most of the stuff (including cmake files)
# be the same throughout platforms.

# target_link_options appeared in 3.13, and is extremely recommended for Android builds
cmake_minimum_required(VERSION 3.13)

project(imgui_demo)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)

# This is an adaptation of the ImGui demo showcasing ImPlot for rendering plots.

set(IMGUI_PATH deps/imgui)
set(IMGUI_IMPL_PATH src/imgui_impl)

# Declare the proper type of target. We'll add sources later.
if (ANDROID)
    # Seems like SDL2 is a bit broken - it wants libhidapi.so, but doesn't build it by default.
    # We'll have to help it out somewhat
    add_library(hidapi SHARED)
    target_sources(hidapi PRIVATE
        deps/SDL/src/hidapi/android/hid.cpp
    )
    target_link_libraries(hidapi PRIVATE
        ${SDL2_LIBRARY} log
    )
    target_include_directories(hidapi PRIVATE
        ${SDL2_INCLUDE_DIR}
    )
    # hid.cpp relies on an internal API call, which is really not necessary
    target_compile_definitions(hidapi PRIVATE
            Android_JNI_RequestPermission=SDL_AndroidRequestPermission)
    add_library(demo SHARED)
    target_link_options(demo PRIVATE
            -Wl,--no-undefined)
    target_link_libraries(demo PRIVATE
            GLESv2
            log
            )
else()
    add_executable(demo)
endif()

# Use GLES2/GLES3 implementations for Android, GL3 for everything else
# We'll get ImGui implementations straight from ImGui distribution

target_sources(demo PRIVATE
    deps/imgui/imgui.cpp
    deps/imgui/imgui_demo.cpp
    deps/imgui/imgui_draw.cpp
    deps/imgui/imgui_tables.cpp
    deps/imgui/imgui_widgets.cpp)

target_include_directories(demo PRIVATE
        deps/imgui)

target_sources(demo PRIVATE
    deps/imgui/backends/imgui_impl_opengl3.cpp
    deps/imgui/backends/imgui_impl_sdl2.cpp)

target_include_directories(demo PRIVATE
    deps/imgui/backends)

if (ANDROID)
    target_compile_definitions(demo PRIVATE "GL_PROFILE_GLES2" "GL_PROFILE_GLES3" "IMGUI_IMPL_OPENGL_ES2")
else()
    target_compile_definitions(demo PRIVATE "GL_PROFILE_GL3")
endif()

target_sources(demo PRIVATE
        src/main.cpp
        deps/implot/implot.cpp
        deps/implot/implot_items.cpp
        deps/easywsclient/easywsclient.cpp)

if (NOT ANDROID)
    target_sources(demo PRIVATE
            src/glload/gl_glcore_3_3.c)
    target_include_directories(demo PRIVATE
            src/glload)
endif()

set(DEMO_SOURCES ${IMGUI_FILES} ${DEMO_FILES} ${GLLOAD_FILES})

target_compile_definitions(demo PRIVATE EASYWSCLIENT_ENABLE_SSL)

find_package(OpenSSL REQUIRED)

if (ANDROID)
    target_link_libraries(demo PRIVATE
            ${SDL2_LIBRARY}
            GLESv2
            OpenSSL::SSL
            OpenSSL::Crypto)
    target_include_directories(demo PRIVATE
            ${SDL2_INCLUDE_DIR}
            deps/implot
            deps
            deps/easywsclient
            ${OPENSSL_INCLUDE_DIR})
else()
    target_link_libraries(demo PRIVATE
            ${SDL2_LIBRARY}
            OpenSSL::SSL
            OpenSSL::Crypto)
    target_include_directories(demo PRIVATE
            ${SDL2_INCLUDE_DIR}
            deps/implot
            deps
            deps/easywsclient
            ${OPENSSL_INCLUDE_DIR})
endif()



