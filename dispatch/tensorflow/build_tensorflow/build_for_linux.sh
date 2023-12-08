compile_host_architecture()
{
   architecture="host"
   echo $architecture
   tflite_path=../../../../claid_external/tensorflow

   current_path=$(pwd)

   cd ${tflite_path}

   path=libs/${architecture}
   mkdir -p ../${path}

   bazel build //tensorflow/lite:tensorflowlite 
   
   rm  -f ../${path}/libtensorflowlite.so
   cp bazel-bin/tensorflow/lite/libtensorflowlite.so ../${path}/libtensorflowlite.so
   cd ${current_path}

   ln -s ${tflite_path} ${current_path}/tensorflow 

}


mkdir libs
compile_host_architecture
