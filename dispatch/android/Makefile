include BuildAndroid.mk


OUTPUT_APK_ANDROID_NAME = claid_capi_android
OUTPUT_APK_ANDROID_PATH = bazel-bin/dispatch/android/
OUTPUT_APK_TMP_PATH_ANDROID = bazel-bin/dispatch/android/extract_libs
TARGET_LIBS_ANDROID = dispatch/android/aar/claid/blobs/

OUTPUT_ANDROID_TFLITE_LIB = claid_capi_android_tflite
WORKSPACE_PATH = ../../

CURRENT_DIR=${PWD}


.PHONY: all
all: build

.PHONY: build_android_libs
build_android_libs: build_android

.PHONY: aar_package
aar_package: build_android_libs
	cd aar/claid && ./gradlew build


.PHONY: gradle_build
gradle_build: 
	cd aar/claid && ./gradlew build

.PHONY: maven_publish
maven_publish: aar_package
	cd aar/claid && ./gradlew publishToMavenLocal


.PHONY: publish_maven
publish_maven: aar_package
	cd aar/claid && ./gradlew publishToMavenLocal 
	cd ~/.m2/ch/claid/claid/0.6 &&
		gpg -ab claid-0.6.aar && 
		gpg -ab claid-0.6.pom && 
		gpg -ab claid-0.6.pom &&

.PHONY: sdk
sdk: 
	rm -fr sdk/**
	mkdir -p sdk/bin/android/native
	mkdir -p sdk/include/dispatch/core
	mkdir -p sdk/include/dispatch/proto
	mkdir -p sdk/include/absl
	mkdir -p sdk/include/google

	cp aar/claid/claid/build/outputs/aar/*.aar sdk/bin/android/
	cp -r ../../bazel-bin/dispatch/android/extract_libs/lib/** sdk/bin/android/native/
	cd ../core && find . -type f \( -name "*.hh" -o -name "*.hpp" \) | tar -cf - --files-from=- | tar -xpf - -C ../android/sdk/include/dispatch/core
	
	cd ../../bazel-bin/dispatch/proto && find . -type f \( -name "*.h" \) | tar -cf - --files-from=- | tar -xpf - -C "${CURRENT_DIR}/sdk/include/dispatch/proto"

	cd ../../bazel-CLAID/external/abseil-cpp+/absl && find . -type f \( -name "*.h" -o  -name "*.inc" \) | tar -cf - --files-from=- | tar -xpf - -C "${CURRENT_DIR}/sdk/include/absl"

	cd ../../bazel-CLAID/external/protobuf+/src/google && find . -type f \( -name "*.h" -o  -name "*.inc" \) | tar -cf - --files-from=- | tar -xpf - -C "${CURRENT_DIR}/sdk/include/google"
