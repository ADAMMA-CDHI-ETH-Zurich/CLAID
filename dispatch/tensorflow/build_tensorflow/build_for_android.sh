cross_compile_android()
{
   architecture=$1
   echo $architecture
   tflite_path=../../../../claid_external/tensorflow

   current_path=$(pwd)

   cd ${tflite_path}

   path=libs/android/${architecture}
   mkdir -p ${path}

   bazel build //tensorflow/lite:tensorflowlite --distinct_host_configuration --crosstool_top=//external:android/crosstool --cpu=$architecture --host_crosstool_top=@bazel_tools//tools/cpp:toolchain 
   
   rm  -f ${path}/libtensorflowlite.so
   cp bazel-bin/tensorflow/lite/libtensorflowlite.so ${path}/libtensorflowlite.so
   cd ${current_path}
}


cross_compile_android "arm64-v8a"
cross_compile_android "armeabi-v7a"
cross_compile_android "x86"
cross_compile_android "x86_64"