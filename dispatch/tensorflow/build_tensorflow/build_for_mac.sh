compile_host_architecture()
{
   architecture="host"
   echo $architecture
   path=libs/${architecture}
   mkdir -p ${path}

   cd tensorflow
   bazel build //tensorflow/lite:tensorflowlite 
   
   rm  -f ../${path}/libtensorflowlite.dylib
   cp bazel-bin/tensorflow/lite/libtensorflowlite.dylib ../${path}/libtensorflowlite.dylib
   cd ..
}


. ./verify_hash.sh

mkdir libs
compile_host_architecture
