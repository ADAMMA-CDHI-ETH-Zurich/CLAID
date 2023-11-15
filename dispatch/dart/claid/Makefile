
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
		TARGET_LIB = dispatch/dart/claid/blobs/libclaid_capi_linux.dylib
    endif
endif

OUTPUT_APK_ANDROID_NAME = claid_capi_android
OUTPUT_APK_ANDROID = bazel-bin/dispatch/android/$(OUTPUT_APK_ANDROID_NAME)
OUTPUT_APK_TMP_PATH_ANDROID = bazel-bin/dispatch/android/extract_libs
TARGET_LIBS_ANDROID = dispatch/dart/claid/blobs/android


.PHONY: all
all: build

.PHONY: clean
clean:
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
	cd ../../..; rm -f $(TARGET_LIB)
	CC=clang bazel build //dispatch/core:claid_capi
	cd ../../..; cp $(OUTPUT_LIB) $(TARGET_LIB)

# Build the Android Apk, extract the libs from the Apk and copy them to the blobs folder.
# Make sure to use CC=gcc, not clang! Otherwise build fails.
.PHONY: build_android 
build_android: copy_android_sources
	cd ../../..; rm -rf $(TARGET_LIBS_ANDROID)
	CC=gcc bazel build //dispatch/android:$(OUTPUT_APK_ANDROID_NAME) --fat_apk_cpu=arm64-v8a,armeabi-v7a
	cd ../../..; rm -fr $(OUTPUT_APK_TMP_PATH_ANDROID)
	cd ../../..; mkdir $(OUTPUT_APK_TMP_PATH_ANDROID)
	cd ../../..; cp $(OUTPUT_APK_ANDROID).apk $(OUTPUT_APK_TMP_PATH_ANDROID)/
	cd ../../..; unzip -o $(OUTPUT_APK_TMP_PATH_ANDROID)/$(OUTPUT_APK_ANDROID_NAME).apk -d $(OUTPUT_APK_TMP_PATH_ANDROID)/
	mkdir -p blobs
	cd ../../..; mkdir $(TARGET_LIBS_ANDROID)
	cd ../../..; cp -r $(OUTPUT_APK_TMP_PATH_ANDROID)/lib/** $(TARGET_LIBS_ANDROID)/

# For the JavaCLAID implementation (i.e., //dispatch/core/java) to be
# available in the flutter plugin, they have to be copied to the android/src/main folder. 
# Sadly, there seems to be no way to just add a reference to the //dispatch/core/java folder.
.PHONY: copy_android_sources
copy_android_sources:
	rm -fr android/src/main/java/adamma/c4dhi/claid
	mkdir -p android/src/main/java/adamma/c4dhi
	cp -r ../../java/adamma/c4dhi/claid android/src/main/java/adamma/c4dhi/claid
	rm -fr android/src/proto
	cp -r ../../proto/ android/src/proto
	rm android/src/proto/BUILD.bazel

.PHONY: generate
generate: generate_dart generate_test_dart generate_ffi

.PHONY: generate_dart
generate_dart:
	mkdir -p lib/generated
	protoc -I../../proto/ --dart_out=grpc:lib/generated \
		../../proto/claidservice.proto \
		google/protobuf/timestamp.proto \
		google/protobuf/empty.proto

.PHONY: generate_test_dart
generate_test_dart:
	mkdir -p test/generated
	protoc -I./test/ --dart_out=grpc:test/generated \
		./test/testmessages.proto

.PHONY: generate_ffi
generate_ffi:
	dart run ffigen --config ffigen.yaml

#	protoc -I../../proto/ --dart_out=grpc:lib/generated ../../proto/claidservice.proto
