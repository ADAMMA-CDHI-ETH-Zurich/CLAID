OUTPUT_APK_ANDROID_NAME = claid_capi_android
OUTPUT_APK_ANDROID_PATH = bazel-bin/dispatch/android/
OUTPUT_APK_TMP_PATH_ANDROID = bazel-bin/dispatch/android/extract_libs
TARGET_LIBS_ANDROID = dispatch/dart/claid/blobs/android

OUTPUT_ANDROID_TFLITE_LIB = claid_capi_android_tflite
WORKSPACE_PATH = ../../../

# Build the Android Apk, extract the libs from the Apk and copy them to the blobs folder.
# Make sure to use CC=gcc, not clang! Otherwise build fails
.PHONY: build_android
build_android: 
	mkdir -p blobs
	cd $(WORKSPACE_PATH); rm -rf $(TARGET_LIBS_ANDROID)
	CC=gcc bazel build //dispatch/android:$(OUTPUT_APK_ANDROID_NAME) --fat_apk_cpu=arm64-v8a,armeabi-v7a,x86,x86_64
	cd $(WORKSPACE_PATH); rm -fr $(OUTPUT_APK_TMP_PATH_ANDROID)
	cd $(WORKSPACE_PATH); mkdir $(OUTPUT_APK_TMP_PATH_ANDROID)
	cd $(WORKSPACE_PATH); cp $(OUTPUT_APK_ANDROID_PATH)/$(OUTPUT_APK_ANDROID_NAME).apk $(OUTPUT_APK_TMP_PATH_ANDROID)/
	cd $(WORKSPACE_PATH); unzip -o $(OUTPUT_APK_TMP_PATH_ANDROID)/$(OUTPUT_APK_ANDROID_NAME).apk -d $(OUTPUT_APK_TMP_PATH_ANDROID)/
	mkdir -p blobs
	cd $(WORKSPACE_PATH); mkdir $(TARGET_LIBS_ANDROID)
	cd $(WORKSPACE_PATH); cp -r $(OUTPUT_APK_TMP_PATH_ANDROID)/lib/** $(TARGET_LIBS_ANDROID)/


# Build the Android Apk with CLAID including tflite, extract the libs from the Apk and copy them to the blobs folder.
# Make sure to use CC=gcc, not clang! Otherwise build fails.
.PHONY: build_android_with_tflite
build_android_with_tflite: 
	mkdir -p blobs
	cd $(WORKSPACE_PATH); rm -rf $(TARGET_LIBS_ANDROID)
	CC=gcc bazel build //dispatch/android:$(OUTPUT_ANDROID_TFLITE_LIB) --fat_apk_cpu=arm64-v8a,armeabi-v7a,x86
	cd $(WORKSPACE_PATH); rm -fr $(OUTPUT_APK_TMP_PATH_ANDROID)
	cd $(WORKSPACE_PATH); mkdir $(OUTPUT_APK_TMP_PATH_ANDROID)
	cd $(WORKSPACE_PATH); cp $(OUTPUT_APK_ANDROID_PATH)/$(OUTPUT_ANDROID_TFLITE_LIB).apk $(OUTPUT_APK_TMP_PATH_ANDROID)/
	cd $(WORKSPACE_PATH); unzip -o $(OUTPUT_APK_TMP_PATH_ANDROID)/$(OUTPUT_ANDROID_TFLITE_LIB).apk -d $(OUTPUT_APK_TMP_PATH_ANDROID)/
	mkdir -p blobs
	cd $(WORKSPACE_PATH); mkdir $(TARGET_LIBS_ANDROID)
	cd $(WORKSPACE_PATH); cp -r $(OUTPUT_APK_TMP_PATH_ANDROID)/lib/** $(TARGET_LIBS_ANDROID)/
	cd $(WORKSPACE_PATH); mv $(TARGET_LIBS_ANDROID)/arm64-v8a/lib$(OUTPUT_ANDROID_TFLITE_LIB).so $(TARGET_LIBS_ANDROID)/arm64-v8a/lib$(OUTPUT_APK_ANDROID_NAME).so 
	cd $(WORKSPACE_PATH); mv $(TARGET_LIBS_ANDROID)/armeabi-v7a/lib$(OUTPUT_ANDROID_TFLITE_LIB).so $(TARGET_LIBS_ANDROID)/armeabi-v7a/lib$(OUTPUT_APK_ANDROID_NAME).so 
	cd $(WORKSPACE_PATH); cp ../claid_external/libs/android/arm64-v8a/* $(TARGET_LIBS_ANDROID)/arm64-v8a/
	cd $(WORKSPACE_PATH); cp ../claid_external/libs/android/armeabi-v7a/* $(TARGET_LIBS_ANDROID)/armeabi-v7a/