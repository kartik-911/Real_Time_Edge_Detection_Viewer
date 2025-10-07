cmake_minimum_required(VERSION 3.22.1)
project("edgedetection")

# Path to OpenCV
set(OpenCV_DIR "/home/kartik/Downloads/opencv-4.12.0-android-sdk/OpenCV-android-sdk/sdk/native/jni" CACHE PATH "Path to OpenCV native jni directory")
find_package(OpenCV REQUIRED)

add_library(edgedetection SHARED native-lib.cpp)

# Android libraries
find_library(log-lib log)
find_library(glesv2-lib GLESv2)

target_link_libraries(
        edgedetection
        ${OpenCV_LIBS}
        ${log-lib}
        ${glesv2-lib}
        -lc++_shared
        -landroid
        -llog
        -lz
        -lm
        -ldl
        # REMOVE -lpthread here, NDK provides it automatically
)
