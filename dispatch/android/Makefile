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
