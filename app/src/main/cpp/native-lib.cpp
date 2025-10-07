#include <jni.h>
#include <android/log.h>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>

#define LOG_TAG "EdgeDetectionNative"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

extern "C"
JNIEXPORT void JNICALL
Java_com_example_edgedetection_MainActivity_processFrame(
        JNIEnv* env,
        jobject /* this */,
        jbyteArray yuvData,
        jint width,
        jint height,
        jobject bitmap) {

    if (yuvData == nullptr || bitmap == nullptr) {
        LOGE("Null input data");
        return;
    }

    jbyte* yuv = env->GetByteArrayElements(yuvData, nullptr);
    if (!yuv) {
        LOGE("Failed to get YUV data");
        return;
    }

    // Convert YUV -> BGR
    cv::Mat yuvMat(height + height / 2, width, CV_8UC1, reinterpret_cast<unsigned char*>(yuv));
    cv::Mat bgrMat;
    try {
        cv::cvtColor(yuvMat, bgrMat, cv::COLOR_YUV2BGR_NV21);
    } catch (const cv::Exception& e) {
        LOGE("OpenCV error: %s", e.what());
        env->ReleaseByteArrayElements(yuvData, yuv, JNI_ABORT);
        return;
    }

    // Edge detection
    cv::Mat edges;
    cv::Canny(bgrMat, edges, 80, 150);

    // Convert edges (grayscale) back to RGBA for drawing
    cv::Mat rgbaMat;
    cv::cvtColor(edges, rgbaMat, cv::COLOR_GRAY2RGBA);

    // Write result to Bitmap
    AndroidBitmapInfo info;
    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) {
        LOGE("Failed to get bitmap info");
        env->ReleaseByteArrayElements(yuvData, yuv, JNI_ABORT);
        return;
    }

    void* pixels = nullptr;
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) {
        LOGE("Failed to lock bitmap pixels");
        env->ReleaseByteArrayElements(yuvData, yuv, JNI_ABORT);
        return;
    }

    // Copy RGBA result to bitmap
    cv::Mat bitmapMat(info.height, info.width, CV_8UC4, pixels);
    if (rgbaMat.size() == bitmapMat.size()) {
        rgbaMat.copyTo(bitmapMat);
    } else {
        cv::resize(rgbaMat, bitmapMat, cv::Size(info.width, info.height));
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    env->ReleaseByteArrayElements(yuvData, yuv, JNI_ABORT);

    LOGD("Frame processed successfully!");
}
