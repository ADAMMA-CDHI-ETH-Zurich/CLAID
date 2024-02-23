#include <jni.h>
#include "dispatch/core/capi.h"
#include <string>
#include <iostream>
static std::string jniStringToStdString(JNIEnv *env, jstring jStr)
{
    if (!jStr)
            return "";

    const jclass stringClass = env->GetObjectClass(jStr);
    const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
    jstring characterSetString =  env->NewStringUTF("UTF-8");
    const jbyteArray stringJbytes = (jbyteArray) env->CallObjectMethod(jStr, getBytes, characterSetString);

    size_t length = (size_t) env->GetArrayLength(stringJbytes);
    jbyte* pBytes = env->GetByteArrayElements(stringJbytes, NULL);

    std::string ret = std::string((char*) pBytes, length);
    env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);

    env->DeleteLocalRef(stringJbytes);
    env->DeleteLocalRef(stringClass);
    env->DeleteLocalRef(characterSetString);
    return ret;
}

static jstring stdStringToJString(JNIEnv* env, const std::string& nativeString)
{
    return env->NewStringUTF(nativeString.c_str());
}

extern "C"
{
    JNIEXPORT void JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_sayHelloDD
  (JNIEnv* env, jobject thisObject) {
    std::cout << "Hello from C++ !!" << std::endl;
}

    
    JNIEXPORT jlong JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_startCore
    (JNIEnv *env, jobject CLAIDOBJ, jstring jsocketPath, jstring jconfigFile, jstring jhostId, jstring juserId, jstring jdeviceId) {
        std::string socketPath = jniStringToStdString(env, jsocketPath);
        std::string configFile = jniStringToStdString(env, jconfigFile);
        std::string hostId = jniStringToStdString(env, jhostId);
        std::string userId = jniStringToStdString(env, juserId);
        std::string deviceId = jniStringToStdString(env, jdeviceId);

        // Call start_core with the C++ std::strings
        void* handle = start_core(socketPath.c_str(), configFile.c_str(), hostId.c_str(), userId.c_str(), deviceId.c_str());
        return reinterpret_cast<jlong>(handle);
    }

    JNIEXPORT void JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_shutdownCore
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        shutdown_core(nativeHandle);
    }

    JNIEXPORT jlong JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_attachCppRuntime
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) 
    {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        return reinterpret_cast<jlong>(attach_cpp_runtime(nativeHandle));
    }

    JNIEXPORT jstring JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_getSocketPath
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        const char* socketPath = get_socket_path(nativeHandle);

        return stdStringToJString(env, socketPath);
    }

    JNIEXPORT jboolean JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeLoadNewConfig
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle, jstring configPath) {

        void* nativeHandle = reinterpret_cast<void*>(handle);

        std::string stdConfigPath = jniStringToStdString(env, configPath);
        return load_new_config(nativeHandle, stdConfigPath.c_str());
    }


    JNIEXPORT void JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_setPayloadDataPath
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle, jstring path) 
    {
        void* nativeHandle = reinterpret_cast<void*>(handle);

        std::string stdPayloadPath = jniStringToStdString(env, path);
        set_payload_data_path(nativeHandle, stdPayloadPath.c_str());
    }

    JNIEXPORT jstring JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_getPayloadDataPath
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) 
    {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        const char* socketPath = get_socket_path(nativeHandle);

        return stdStringToJString(env, socketPath);
    }

    JNIEXPORT void JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_enableDesignerMode
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle, jstring path) 
    {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        enable_designer_mode(nativeHandle);
    }

    __attribute__((visibility("default"))) __attribute__((used))
    JNIEXPORT void JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_disableDesignerMode
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) 
    {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        disable_designer_mode(nativeHandle);
    }
}
