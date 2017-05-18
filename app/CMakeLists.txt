# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.4.1)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/include/
            /home/chris/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/lib64/clang/3.8/include/)

IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    SET(CMAKE_CXX_FLAGS "-lz -ldl -llog -O0 -ggdb -imacros ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/imacros.h -std=gnu++1z -fpermissive -w -mfpu=neon -ffunction-sections -fdata-sections -fvisibility=hidden -fstrict-aliasing -integrated-as -march=armv7-a -fms-extensions -fno-rtti -fno-exceptions")
    SET(CMAKE_C_FLAGS "-O0 -ggdb")
else(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    return()
endif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")

#set (CMAKE_SHARED_LINKER_FLAGS "-flto")

add_library( # Specifies the name of the library.
             glitch-native

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/cpp/GlitchAlgo.cpp
             src/cpp/pixelsort.cpp
             src/cpp/NativelyCachedBmp.cpp
             src/cpp/InitialSeed.cpp
             src/cpp/CSApplication.cpp
             src/cpp/JavaClass.cpp
             src/cpp/JClassLoader.cpp

             src/cpp/CSPrefs.cpp
             src/cpp/uncompressClassname.cpp
             src/cpp/VideoBender.cpp
             src/cpp/FileUtils.cpp
             src/cpp/JNIUtils.cpp
             src/cpp/BitmapUtils.cpp
             src/cpp/GlobalError.cpp
             src/cpp/include/util/Vector_t.cpp
             src/cpp/ArmEmitter.cpp
             src/cpp/SharedLibrary.cpp
             src/cpp/AndroidCl.cpp
             src/cpp/ffmpeg.cpp
             src/cpp/ffmpeg_opt.cpp
             src/cpp/ffmpeg_filter.cpp
             src/cpp/cmdutils.cpp
             src/cpp/include/util/string_t.cpp)

set(LIBS_SAFE "${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/libs_safe")
set(LIBS_OPT "${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/libs_optimized")
set(LIBCHOICE ${LIBS_SAFE})
TARGET_LINK_LIBRARIES(glitch-native
    ${LIBCHOICE}/libavformat.a
    ${LIBCHOICE}/libavdevice.a
    ${LIBCHOICE}/libavfilter.a
    ${LIBCHOICE}/libavcodec.a
    ${LIBCHOICE}/libavutil.a
    ${LIBCHOICE}/libswresample.a
    ${LIBCHOICE}/libswscale.a
    ${LIBCHOICE}/libx264.a
    ${LIBCHOICE}/fast_qsort.a
    ${LIBCHOICE}/libmp3lame.a)
    #    ${LIBCHOICE}/ffmpeg.a
