
ifeq ($(OS),Windows_NT)
    OUTPUT_LIB = bazel-bin/dispatch/core/libclaid_capi.dll
	TARGET_LIB = dispatch/dart/claid/blobs/libclaid_capi_linux.dll
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        OUTPUT_LIB = bazel-bin/dispatch/core/libclaid_capi.so
		TARGET_LIB = dispatch/dart/claid/blobs/libclaid_capi_linux.so
    endif
    ifeq ($(UNAME_S),Darwin)
        OUTPUT_LIB = bazel-bin/dispatch/core/libclaid_capi.dylib
		TARGET_LIB = dispatch/dart/claid/blobs/libclaid_capi.dylib
    endif
endif

OUTPUT_APK_ANDROID_NAME = claid_capi_android
OUTPUT_APK_ANDROID_PATH = bazel-bin/dispatch/android/
OUTPUT_APK_TMP_PATH_ANDROID = bazel-bin/dispatch/android/extract_libs
TARGET_LIBS_ANDROID = dispatch/dart/claid/blobs/android

OUTPUT_ANDROID_TFLITE_LIB = claid_capi_android_tflite

PROTO_DIR = ../../../dispatch/proto
PROTO_FILES = $(shell find $(PROTO_DIR) -type f -name "*.proto")

.PHONY: all
all: build

.PHONY: clean
clean:
	mkdir -p blobs
	rm -f blobs/*.so

# Build everything and run a series of tests.
.PHONY: test
test: build
	flutter test ./test/

# Run the tests, gather coverage and generate a report.
.PHONY: coverage
coverage: build
	mkdir -p coverage
	flutter test --coverage ./test/
	rm -rf coverage/output/*
	mkdir -p coverage/output
	genhtml -o coverage/output/ coverage/lcov.info

# Build the middleware and generate the dart code.
.PHONY: build
build: build_middleware generate

# Build the middleware via bazel.
.PHONY: build_middleware
build_middleware:
	mkdir -p blobs
	cd ../../..; rm -f $(TARGET_LIB)
	CC=clang bazel build //dispatch/core:claid_capi
	cd ../../..; cp $(OUTPUT_LIB) $(TARGET_LIB)

# Build the Android Apk, extract the libs from the Apk and copy them to the blobs folder.
# Make sure to use CC=gcc, not clang! Otherwise build fails
.PHONY: flutter_package
flutter_package: build
	rm -fr android/libs
	mkdir android/libs
	cd ../../android/; make aar_package && cp aar/claid/claid/build/outputs/aar/claid-debug.aar ../dart/claid/android/libs/claid_debug.aar

flutter_package_host_only:  build
	rm -fr android/libs
	





# mkdir -p blobs
# cd ../../..; rm -rf $(TARGET_LIBS_ANDROID)
# CC=gcc bazel build //dispatch/android:$(OUTPUT_APK_ANDROID_NAME) --fat_apk_cpu=arm64-v8a,armeabi-v7a
# cd ../../..; rm -fr $(OUTPUT_APK_TMP_PATH_ANDROID)
# cd ../../..; mkdir $(OUTPUT_APK_TMP_PATH_ANDROID)
# cd ../../..; cp $(OUTPUT_APK_ANDROID_PATH)/$(OUTPUT_APK_ANDROID_NAME).apk $(OUTPUT_APK_TMP_PATH_ANDROID)/
# cd ../../..; unzip -o $(OUTPUT_APK_TMP_PATH_ANDROID)/$(OUTPUT_APK_ANDROID_NAME).apk -d $(OUTPUT_APK_TMP_PATH_ANDROID)/
# mkdir -p blobs
# cd ../../..; mkdir $(TARGET_LIBS_ANDROID)
# cd ../../..; cp -r $(OUTPUT_APK_TMP_PATH_ANDROID)/lib/** $(TARGET_LIBS_ANDROID)/


# # Build the Android Apk with CLAID including tflite, extract the libs from the Apk and copy them to the blobs folder.
# # Make sure to use CC=gcc, not clang! Otherwise build fails.
# .PHONY: build_android_with_tflite_libs
# build_android_with_tflite: copy_android_sources
# 	mkdir -p blobs
# 	cd ../../..; rm -rf $(TARGET_LIBS_ANDROID)
# 	CC=gcc bazel build //dispatch/android:$(OUTPUT_ANDROID_TFLITE_LIB) --fat_apk_cpu=arm64-v8a,armeabi-v7a
# 	cd ../../..; rm -fr $(OUTPUT_APK_TMP_PATH_ANDROID)
# 	cd ../../..; mkdir $(OUTPUT_APK_TMP_PATH_ANDROID)
# 	cd ../../..; cp $(OUTPUT_APK_ANDROID_PATH)/$(OUTPUT_ANDROID_TFLITE_LIB).apk $(OUTPUT_APK_TMP_PATH_ANDROID)/
# 	cd ../../..; unzip -o $(OUTPUT_APK_TMP_PATH_ANDROID)/$(OUTPUT_ANDROID_TFLITE_LIB).apk -d $(OUTPUT_APK_TMP_PATH_ANDROID)/
# 	mkdir -p blobs
# 	cd ../../..; mkdir $(TARGET_LIBS_ANDROID)
# 	cd ../../..; cp -r $(OUTPUT_APK_TMP_PATH_ANDROID)/lib/** $(TARGET_LIBS_ANDROID)/
# 	cd ../../..; mv $(TARGET_LIBS_ANDROID)/arm64-v8a/lib$(OUTPUT_ANDROID_TFLITE_LIB).so $(TARGET_LIBS_ANDROID)/arm64-v8a/lib$(OUTPUT_APK_ANDROID_NAME).so 
# 	cd ../../..; mv $(TARGET_LIBS_ANDROID)/armeabi-v7a/lib$(OUTPUT_ANDROID_TFLITE_LIB).so $(TARGET_LIBS_ANDROID)/armeabi-v7a/lib$(OUTPUT_APK_ANDROID_NAME).so 
# 	cd ../../..; cp ../claid_external/libs/android/arm64-v8a/* $(TARGET_LIBS_ANDROID)/arm64-v8a/
# 	cd ../../..; cp ../claid_external/libs/android/armeabi-v7a/* $(TARGET_LIBS_ANDROID)/armeabi-v7a/
# # For the JavaCLAID implementation (i.e., //dispatch/core/java) to be
# # available in the flutter plugin, they have to be copied to the android/src/main folder.
# # Sadly, there seems to be no way to just add a reference to the //dispatch/core/java folder.
# .PHONY: copy_android_sources
# copy_android_sources:
# 	rm -fr android/src/main/java/adamma/c4dhi/claid
# 	mkdir -p android/src/main/java/adamma/c4dhi
# 	cp -r ../../java/runtime/adamma/c4dhi/claid android/src/main/java/adamma/c4dhi/claid
# 	cp -r ../../android/platform/adamma/c4dhi/** android/src/main/java/adamma/c4dhi/
# 	rm -fr android/src/proto
# 	cp -r ../../proto/ android/src/proto
# 	rm android/src/proto/BUILD.bazel

.PHONY: generate
generate: generate_dart generate_test_dart generate_ffi

.PHONY: generate_dart
generate_dart:
	mkdir -p lib/generated
	protoc -I../../proto/ --dart_out=grpc:lib/generated \
		../../proto/claidservice.proto \
		../../proto/*.proto\
		google/protobuf/timestamp.proto \
		google/protobuf/empty.proto \
		google/protobuf/struct.proto 


.PHONY: generate_test_dart
generate_test_dart:
	mkdir -p test/generated
	protoc -I./test/ --dart_out=grpc:test/generated \
		./test/testmessages.proto

.PHONY: generate_ffi
generate_ffi:
	dart run ffigen --config ffigen.yaml

#	protoc -I../../proto/ --dart_out=grpc:lib/generated ../../proto/claidservice.proto
