/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#include <jni.h>
#include "dispatch/core/capi.h"
#include "dispatch/core/Logger/Logger.hh"
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

    JNIEXPORT jlong JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_startCoreWithEventTracker
    (JNIEnv *env, jobject CLAIDOBJ, jstring jsocketPath, jstring jconfigFile, jstring jhostId, jstring juserId, jstring jdeviceId, jstring jcommonDataPath) {
        std::string socketPath = jniStringToStdString(env, jsocketPath);
        std::string configFile = jniStringToStdString(env, jconfigFile);
        std::string hostId = jniStringToStdString(env, jhostId);
        std::string userId = jniStringToStdString(env, juserId);
        std::string deviceId = jniStringToStdString(env, jdeviceId);
        std::string commonDataPath = jniStringToStdString(env, jcommonDataPath);

        // Call start_core with the C++ std::strings
        void* handle = start_core_with_event_tracker(socketPath.c_str(), configFile.c_str(), hostId.c_str(), userId.c_str(), deviceId.c_str(), commonDataPath.c_str());
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

    JNIEXPORT jstring JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeGetSocketPath
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        const char* socketPath = get_socket_path(nativeHandle);

        return stdStringToJString(env, socketPath);
    }

    JNIEXPORT jstring JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeGetHostId
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        const char* hostId = get_host_id(nativeHandle);

        return stdStringToJString(env, hostId);
    }

    JNIEXPORT jstring JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeGetUserId
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        const char* userId = get_user_id(nativeHandle);

        return stdStringToJString(env, userId);
    }

    JNIEXPORT jstring JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeGetDeviceId
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        const char* deviceId = get_device_id(nativeHandle);

        return stdStringToJString(env, deviceId);
    }

    JNIEXPORT jboolean JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeLoadNewConfig
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle, jstring configPath) {

        void* nativeHandle = reinterpret_cast<void*>(handle);

        std::string stdConfigPath = jniStringToStdString(env, configPath);
        return load_new_config(nativeHandle, stdConfigPath.c_str());
    }

    JNIEXPORT void JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeSetPayloadDataPath
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle, jstring path) 
    {
        void* nativeHandle = reinterpret_cast<void*>(handle);

        std::string stdPayloadPath = jniStringToStdString(env, path);
        set_payload_data_path(nativeHandle, stdPayloadPath.c_str());
    }

    JNIEXPORT jstring JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeGetPayloadDataPath
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) 
    {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        const char* path = get_payload_data_path(nativeHandle);

        return stdStringToJString(env, path);
    }

    JNIEXPORT void JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeSetCommonDataPath
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle, jstring path) 
    {
        void* nativeHandle = reinterpret_cast<void*>(handle);

        std::string stdCommonDataPath = jniStringToStdString(env, path);
        set_common_data_path(nativeHandle, stdCommonDataPath.c_str());
    }

    JNIEXPORT jstring JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeGetCommonDataPath
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) 
    {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        const char* path = get_common_data_path(nativeHandle);

        return stdStringToJString(env, path);
    }

    JNIEXPORT void JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeEnableDesignerMode
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) 
    {
        claid::Logger::logInfo("native java enable designer mode called");

        void* nativeHandle = reinterpret_cast<void*>(handle);
        enable_designer_mode(nativeHandle);
    }

    JNIEXPORT void JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeDisableDesignerMode
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) 
    {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        disable_designer_mode(nativeHandle);
    }

    JNIEXPORT jint JNICALL Java_adamma_c4dhi_claid_JavaCLAIDBase_nativeGetLogSinkSeverityLevel
    (JNIEnv *env, jobject CLAIDOBJ, jlong handle) 
    {
        void* nativeHandle = reinterpret_cast<void*>(handle);
        return get_log_sink_severity_level(nativeHandle);
    }
}
