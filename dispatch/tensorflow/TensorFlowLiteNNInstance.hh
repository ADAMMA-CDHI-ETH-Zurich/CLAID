

#ifndef HEADER_FILES_TensorFlowLiteNNInstance_H_
#define HEADER_FILES_TensorFlowLiteNNInstance_H_


#include <cstdint>

#include <string>



#include "tensorflow/lite/interpreter.h"

#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tensorflow/lite/string_util.h"

#include "dispatch/proto/layerdata.pb.h"

using namespace claid;


#ifdef __ANDROID__
#include "TensorFlowAndroidJNIUtils.hh"
#endif

#include "dispatch/core/Logger/Logger.hh"

//#define __TFNN_USE_GPU 0

#if __TFNN_USE_GPU == 1
	#include "tensorflow/lite/delegates/gpu/gl_delegate.h"
	#define TFLITE_USE_GPU_DELEGATE 1
#endif



namespace claid
{
	
    /**
     * Class that conventiently allows to manage tensorflowlite
     * neural network instances without the need to use tflite API directly.
     * 
     * Functionalities: 
     * Allows to load and instantiate saved models from files (for android, loading from assets is supported).
     * Allows applying input on the network as well as retrieving it's output.
     * Running inference.
     * Enabling GPU support, specifiying number of threads used for computation.
     * Retrieve information about individual layers.
     * 
     * For each function, a sophisticated error management is implemented.
     * Whenever any error occures, the last error is stored
     * and can be retrieved externally, hopefully providing 
     * information about what went wrong (e.g. model file invalid, out of memory,
     * unsupported types, etc.).
     * 
     */
    class TensorFlowLiteNNInstance
    {

        public:

            enum class ErrorType
            {
                ERROR_NONE,

                // If specified file does not exist.
                ERROR_INVALID_MODEL_FILE,

                // If the given file is of wrong format (i.e. not *.tflite)
                ERROR_COULD_NOT_MMAP_MODEL,
                ERROR_FAILED_TO_CONSTRUCT_INTERPRETER,

                // If the given network model uses an input type
                // we don't support / did not consider.
                ERROR_NN_UNSUPPORTED_INPUT_TYPE,

                // If the given network model uses an output type
                // we don't support / did not consider.
                ERROR_NN_UNSUPPORTED_OUTPUT_TYPE,

                // For unsupported data types in general (independent whether a layer is input or output).
                ERROR_NN_UNSUPPORTED_DATA_TYPE,

                // If interpreter.AllocateTensors() fails
                ERROR_NN_FAILED_TO_ALLOCATE_TENSORS,

                // If interpreter.invoke() fails
                ERROR_NN_INVOKE_FAILED,

                // If number of input bytes for an layer doesn't match the expected number of bytes.
                ERROR_APPLY_INPUT_BYTE_MISMATCH,

                // If a name for an input layer was given that we don't know (e.g. in getInputIDFromLayerName(...)).
                ERROR_CANNOT_FIND_INPUT_LAYER_ID_BY_NAME,

                // If ModifyGraphWithDelegate() failed
                #if __TFNN_USE_GPU == 1
                ERROR_MODIFY_GRAPH_WITH_DELEGATE_GPU,
                #endif
                // ====== TensorflowNNIntegrationClassifier ====== //

                // If the specified labels file does not exist
                ERROR_TFNN_CLASSIFIER_INVALID_LABELS_FILE,

                #ifdef __ANDROID__
                // ====== Android specific Errors ==== //

                // If a model file could not be loaded from assets.
                ERROR_LOAD_MODEL_FROM_ASSETS_FAILED,
                #endif
            };

        private:

            uint8_t* inputVector;

            #ifdef __ANDROID__
                char* flatBuffersBuffer = NULL;
            #endif

        protected:
            std::unique_ptr<tflite::FlatBufferModel> model;
            std::unique_ptr<tflite::Interpreter> tfLiteInterpreter;
            tflite::ops::builtin::BuiltinOpResolver resolver;

            #if __TFNN_USE_GPU == 1
                TfLiteDelegate* gpuDelegate;
            #endif

            LayerDataOrder networkLayerDataOrder;

            ErrorType lastError;

            bool fileExists(const std::string& name);





        public:

            const tflite::Interpreter* getInterpreterPtr();


            bool setupModelFromFile(const std::string& modelFile, LayerDataOrder networkLayerDataOrder, const int numThreads = 1);

            #ifdef __ANDROID__
                bool setupModelFromAssets(AAssetManager* const assetManager, const std::string& modelFile, LayerDataOrder networkLayerDataOrder, const int numThreads = 1);
            #endif

            bool runInference();

            bool applyInput(const size_t inputID, const void* data, const size_t numBytes);
            bool applyInput(const LayerData& layerData);

            size_t getLayerDataNumBytes(const LayerData& layerData);

            void getDimensionVectorFromTensor(TfLiteTensor* tensor, std::vector<int64_t>& dimensions);

            int getInputIDFromLayerName(const std::string& layerName);
            int getOutputIDFromLayerName(const std::string& layerName);

            bool layerDataTypeFromTensor(TfLiteTensor* tensor, LayerDataType& layerDataType);
            bool getLayerDataTypeNumBytesFromTensor(TfLiteTensor* tensor, size_t& numBytes);
            bool getOutputLayerData(LayerDataVector& layerDataVector);

            bool getInputLayerNumBytes(const size_t inputID, size_t& dimensions);
            bool getOutputLayerNumBytes(const size_t inputID, size_t& dimensions);


            ErrorType getLastError() const;
            std::string getLastErrorString() const;


            TensorFlowLiteNNInstance();
            ~TensorFlowLiteNNInstance();

            // Need to integrate template function to this header:
            template<typename T>
            T* getInputArray()
            {
                int input = this->tfLiteInterpreter->inputs()[0];
                return this->tfLiteInterpreter->typed_tensor<T>(input);
            }


    };
}



#endif /* HEADER_FILES_TensorFlowLiteNNInstance_H_ */