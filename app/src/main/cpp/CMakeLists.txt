cmake_minimum_required(VERSION 3.22.1)
project("edgedetection")

# --- IMPORTANT: PASTE YOUR CORRECT PATH HERE ---
set(OpenCV_DIR "/home/kartik/Downloads/opencv-4.12.0-android-sdk/OpenCV-android-sdk/sdk/native/jni" CACHE PATH "Path to OpenCV native jni directory")
find_package(OpenCV REQUIRED)

add_library(edgedetection SHARED native-lib.cpp)

find_library(log-lib log)

target_link_libraries(
        edgedetection
        ${OpenCV_LIBS}
        log
)