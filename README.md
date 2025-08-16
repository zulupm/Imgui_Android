# Imgui_Android
A demo of dear imgui running on Android
![screenshot](https://raw.githubusercontent.com/sfalexrog/Imgui_Android/master/screenshot/screenshot.png)

The running build number is displayed in the corner for quick version identification.

## What?

This is a port of [dear imgui](https://github.com/ocornut/imgui) SDL2 example to Android. It now showcases [ImPlot](https://github.com/epezent/implot) by streaming Bitcoin and Ethereum prices from the [Binance WebSocket API](https://binance.com/en/binance-api) and plotting them in real time. A built-in log window reports runtime errors, connection traces, and incoming API messages.

The demo requests Internet permission so it can reach the public WebSocket endpoints for live price data.

As an added bonus, you can compile and run this demo on a desktop! (Tested only on Linux so far)

## How?

This demo, apart from dear imgui, uses [SDL2](http://libsdl.org/), [stb_image](https://github.com/nothings/stb), [ImPlot](https://github.com/epezent/implot), and [easywsclient](https://github.com/dhbaird/easywsclient) to stream cryptocurrency data from Binance. It includes SDL as a submodule. You don't need SDL2 sources if you're
building on a desktop, though, but you'll need SDL2 installed.

I ran into some issues while working on this demo. First, Android's virtual keyboard sends events in a weird way, making
backspace, for instance, not working or working unreliably. This doesn't concern actual physical keyboards, but since the
vast majority of Android users don't have a hardware keyboard, I've made a hack that discards the keyUp event for Backspace
until the rendering starts. This may or may not work with your setup, and may or may not work with hardware keyboards.

Second, since I'm buiding the native part with CMake, I've decided to try and use SDL's CMakeLists.txt directly. I had to make
some modifications to it in order for it to work, but it seems to build just fine. In order to keep instances of `if(ANDROID)` 
in my own CMakeLists.txt, I've decided to hack a FindSDL2.cmake module I've found. So now `find_package(SDL2)` will build
SDL2 if you're building for Android and will try to find SDL2 libraries on a desktop. Not sure if that's a good idea (please
tell me why if it isn't!), but it seems to work.

## Build

### Android

You'll need OpenJDK 1.8, Android SDK, NDK, and Android-specific cmake in order to build this demo. Good thing is, you only really need the
SDK and NDK parts, gradle will install everything else for you if you accept the licenses. In order to accept the licenses, run

    ${ANDROID_SDK_ROOT}/tools/bin/sdkmanager --licenses

and accept the licenses. If you don't have an NDK, run

    ${ANDROID_SDK_ROOT}/tools/bin/sdkmanager ndk-bundle

This will download the latest NDK and put it into `${ANDROID_SDK_ROOT}/ndk-bundle`.

Then, download the project and its submodules:

    $ git clone --recursive https://github.com/sfalexrog/Imgui_Android

(or do `git submodule update --init --recursive` after cloning if your Git client does not support the `--recursive` option)

Finally, run

    $ ./gradlew assembleDebug
    
in the project folder. Gradle wrapper will download the required gradle version, and gradle plugin will download everything
that's required. Your apk will be in `ImguiDemoSdl/build/outputs/apk`.

Of course, you can simply run Android Studio and open this project. Android Studio will prompt you to install all missing
components.

### Desktop (Linux, really)

You won't need all submodules, glm and imgui will be enough. Unfortunately I'm not using system glm. You'll have to clone the
project without submodules, and then only initialize glm and imgui:

    $ git clone https://github.com/sfalexrog/Imgui_Android
    $ cd Imgui_Android
    $ git submodule update --init --recursive ImguiDemoSdl/src/main/cpp/deps/glm
    $ git submodule update --init --recursive ImguiDemoSdl/src/main/cpp/deps/imgui
    
Then, create a build directory somewhere out-of-source, and run

    $ cmake ${PATH_TO_IMGUI_ANDROID}/ImguiDemoSdl/src/main/cpp
    $ make demo
    $ ./demo ${PATH_TO_IMGUI_ANDROID}/ImguiDemoSdl/src/main/cpp/data

That's basically it. You should have a demo on your screen.

## Why?

I've done this project mostly to create a correct ES2 implementation for ImGui, but also to try and write cross-platform
C++/OpenGL code that will use the same set of build files on Android and desktop. Oh, and for the fun of it!

## Next?

Since this is more of a proof-of-concept (well, maybe several concepts), there are a lot of things that are worth doing:

 - Since this is a working implementation of ImGui and ImPlot for OpenGL ES2, and a working project with SDL2, you can try adding your own game/app here and have a nice overlay!
