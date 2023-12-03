include BuildAndroid.mk


OUTPUT_APK_ANDROID_NAME = claid_capi_android
OUTPUT_APK_ANDROID_PATH = bazel-bin/dispatch/android/
OUTPUT_APK_TMP_PATH_ANDROID = bazel-bin/dispatch/android/extract_libs
TARGET_LIBS_ANDROID = dispatch/android/aar/claid/blobs/

OUTPUT_ANDROID_TFLITE_LIB = claid_capi_android_tflite
WORKSPACE_PATH = ../../

.PHONY: all
all: build

.PHONY: build_android_libs
build_android_libs: build_android