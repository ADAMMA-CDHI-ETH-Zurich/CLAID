
if grep -q '^android_sdk_repository' WORKSPACE.bazel && grep -q '^android_ndk_repository' WORKSPACE.bazel; then
    echo "Android is already configured in WORKSPACE file."
else
    echo "android_sdk_repository(
        name = \"androidsdk\", # Required. Name *must* be \"androidsdk\".
    )

android_ndk_repository(
        name = \"androidndk\", # Required. Name *must* be \"androidndk\".
    )" >> WORKSPACE.bazel
fi