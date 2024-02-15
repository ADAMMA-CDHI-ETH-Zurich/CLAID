compile_host_architecture()
{
   architecture="host"
   echo $architecture
   tflite_path=../../../../claid_external/tensorflow

   current_path=$(pwd)

   cd ${tflite_path}

   path=libs/${architecture}
   mkdir -p ${current_path}/${path}

   bazel build //tensorflow/lite:tensorflowlite 
   
   rm  -f ${current_path}/${path}/libtensorflowlite.dylib
   cp bazel-bin/tensorflow/lite/libtensorflowlite.dylib ${current_path}/${path}/libtensorflowlite.dylib
   cd ${current_path}

  # ln -s ${tflite_path} ${current_path}/tensorflow 

}


mkdir libs
compile_host_architecture
