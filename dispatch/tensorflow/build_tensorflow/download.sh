
tflite_path=../../../../claid_external/tensorflow

mkdir -p ${tflite_path}
git clone https://github.com/tensorflow/tensorflow.git ${tflite_path}

cd ${tflite_path}
git checkout v2.11.0
./configure
