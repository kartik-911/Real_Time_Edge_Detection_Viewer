#include <jni.h>
#include <android/log.h>
#include <mutex>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cstdlib>

#define LOG_TAG "Native-Lib"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global variables
cv::Mat processedMat;
std::mutex frameMutex;
GLuint gProgram = 0;
GLuint gTextureId = 0;
GLuint gPositionLoc = 0;
GLuint gTexCoordLoc = 0;

// Vertex shader
const char* VERTEX_SHADER =
        "attribute vec4 a_position;\n"
        "attribute vec2 a_texCoord;\n"
        "varying vec2 v_texCoord;\n"
        "void main() {\n"
        "  gl_Position = a_position;\n"
        "  v_texCoord = a_texCoord;\n"
        "}\n";

// Fragment shader
const char* FRAGMENT_SHADER =
        "precision mediump float;\n"
        "varying vec2 v_texCoord;\n"
        "uniform sampler2D s_texture;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(s_texture, v_texCoord);\n"
        "}\n";

// Compile a shader
GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, nullptr);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, nullptr, buf);
                    LOGE("Could not compile shader %d:\n%s\n", shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

// Create OpenGL program
GLuint createProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
    if (!vertexShader) return 0;
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fragmentShader) return 0;

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, nullptr, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

// Process Camera frame (NV21)
extern "C" JNIEXPORT void JNICALL
Java_com_example_edgedetection_MainActivity_processFrame(
        JNIEnv* env,
        jobject,
        jbyteArray yuv_data,
        jint width,
        jint height) {

    jbyte* yuv = env->GetByteArrayElements(yuv_data, nullptr);

    // Convert NV21 to gray
    cv::Mat yuvMat(height + height / 2, width, CV_8UC1, yuv);
    cv::Mat grayMat;
    cv::cvtColor(yuvMat, grayMat, cv::COLOR_YUV2GRAY_NV21);

    // Apply Canny edge detection
    cv::Mat edgesMat;
    cv::Canny(grayMat, edgesMat, 100, 200);

    // Rotate + flip to correct portrait, vertical, and horizontal mirroring
    cv::Mat rotated;
    cv::transpose(edgesMat, rotated);
    cv::flip(rotated, rotated, -1); // flip both vertically and horizontally

    std::lock_guard<std::mutex> lock(frameMutex);
    cv::cvtColor(rotated, processedMat, cv::COLOR_GRAY2RGBA);

    env->ReleaseByteArrayElements(yuv_data, yuv, 0);
}

// OpenGL callbacks
extern "C" JNIEXPORT void JNICALL
Java_com_example_edgedetection_MyGLRenderer_onSurfaceCreated(JNIEnv*, jobject) {
    gProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (!gProgram) {
        LOGE("Could not create GL program");
        return;
    }

    gPositionLoc = glGetAttribLocation(gProgram, "a_position");
    gTexCoordLoc = glGetAttribLocation(gProgram, "a_texCoord");

    glGenTextures(1, &gTextureId);
    glBindTexture(GL_TEXTURE_2D, gTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_edgedetection_MyGLRenderer_onSurfaceChanged(JNIEnv*, jobject, jint width, jint height) {
    glViewport(0, 0, width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_edgedetection_MyGLRenderer_onDrawFrame(JNIEnv*, jobject) {
    glClear(GL_COLOR_BUFFER_BIT);

    std::lock_guard<std::mutex> lock(frameMutex);
    if (!processedMat.empty()) {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, processedMat.cols, processedMat.rows,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, processedMat.data);

        glUseProgram(gProgram);

        GLfloat vertices[] = {
                -1.0f, -1.0f,
                1.0f, -1.0f,
                -1.0f,  1.0f,
                1.0f,  1.0f
        };

        GLfloat textureCoords[] = {
                0.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f
        };

        glVertexAttribPointer(gPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(gTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, textureCoords);

        glEnableVertexAttribArray(gPositionLoc);
        glEnableVertexAttribArray(gTexCoordLoc);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(gPositionLoc);
        glDisableVertexAttribArray(gTexCoordLoc);
    }
}
