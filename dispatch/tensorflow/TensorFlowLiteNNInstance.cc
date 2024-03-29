/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/



#include "dispatch/tensorflow/TensorFlowLiteNNInstance.hh"
#include <cstdio>

#include <iostream>


#include <stdlib.h>


#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <sys/stat.h>


#ifdef __ANDROID__
	#include <android/asset_manager.h>
	#include <android/asset_manager_jni.h>
	#include <jni.h>
#endif



/**
 * Constructor of TensorFlowLiteNNInstance.
 * 
 * 
 */
claid::TensorFlowLiteNNInstance::
	TensorFlowLiteNNInstance():
	inputVector(NULL), lastError(ErrorType::ERROR_NONE)
{
#ifdef __ANDROID__
    this->flatBuffersBuffer = NULL;
#endif
}

/**
 * @brief Parses a tensorflow lite model file and
 * instantiates a tensorflowlite interpreter.
 * 
 * @param modelFile Path to a tensorflow lite model file.
 * @param networkLayerDataOrder Data order of the network, e.g. NCHW or HWC
 * @param numThreads Number of threads to use for inference
 * @return true if network was loaded and instantiated successfully, false otherwise (e.g.
 * model file does not exist or is invalid).
 */
bool claid::TensorFlowLiteNNInstance::setupModelFromFile(const std::string& modelFile, LayerDataOrder networkLayerDataOrder, const int numThreads)
{
	this->networkLayerDataOrder = networkLayerDataOrder;
    claid::Logger::logInfo("Checking if file exists");
	if(fileExists(modelFile))
	{
        claid::Logger::logInfo("Building model");
		this->model =
				tflite::FlatBufferModel::BuildFromFile(modelFile.c_str(),
						NULL);


		if (this->model)
		{
            claid::Logger::logInfo("Building interpreter");
			this->model->error_reporter();

			tflite::InterpreterBuilder(*model, this->resolver)(&this->tfLiteInterpreter);

			if (this->tfLiteInterpreter)
			{
				this->tfLiteInterpreter->SetNumThreads(numThreads);
				//this->tfLiteInterpreter->UseNNAPI(this->instanceSettings.useNNAPI);

				#if __TFNN_USE_GPU == 1
				    claid::Logger::logInfo("GPU support is available %s", this->modelFile.c_str());

					if(this->instanceSettings.useGPU)
					{
						this->gpuDelegate = TfLiteGpuDelegateCreate(nullptr);  // default config
											
						claid::Logger::logInfo("Modifiying Graph to enable GPU support.");

               				if (this->tfLiteInterpreter->ModifyGraphWithDelegate(this->gpuDelegate) != kTfLiteOk)
						{
							claid::Logger::logInfo("Failed to ModifyGraphWithDelegate, cannot use GPU.");
							this->lastError = TensorFlowLiteNNInstance::ErrorType::
									ERROR_MODIFY_GRAPH_WITH_DELEGATE_GPU;
							return false;
						}
						claid::Logger::logInfo("GPU support enabled.");
					}
				#endif
				if (this->tfLiteInterpreter->AllocateTensors() == kTfLiteOk)
				{
                    claid::Logger::logInfo("tensors size: %d", this->tfLiteInterpreter->tensors_size());
                    claid::Logger::logInfo("nodes size: %d", this->tfLiteInterpreter->nodes_size());
                    claid::Logger::logInfo("inputs: %d", this->tfLiteInterpreter->inputs().size());
                    claid::Logger::logInfo("input(0) name: %s", this->tfLiteInterpreter->GetInputName(0));


					return true;
				}
				else
				{
                    claid::Logger::logInfo("Failed to allocate tensors");
					this->lastError = TensorFlowLiteNNInstance::ErrorType::
							ERROR_NN_FAILED_TO_ALLOCATE_TENSORS;
					return false;
				}

			}
			else
			{
                claid::Logger::logInfo("Interpreter fail");
				this->lastError = TensorFlowLiteNNInstance::ErrorType::
						ERROR_FAILED_TO_CONSTRUCT_INTERPRETER;
				return false;
			}
		  }
		  else
		  {
			  this->lastError = TensorFlowLiteNNInstance::ErrorType::
					  ERROR_COULD_NOT_MMAP_MODEL;
			  return false;
		  }
	}
	else
	{
		this->lastError = TensorFlowLiteNNInstance::ErrorType::
				ERROR_INVALID_MODEL_FILE;
		return false;
	}

}

#ifdef __ANDROID__
/**
 * @brief Parses a tensorflow lite model file and
 * instantiates a tensorflowlite interpreter.
 * In contrast to setupFromModelFile(...), this function
 * loads the model file from an android app's assets directory.
 * 
 * @param modelFile Path to a tensorflow lite model file.
 * @param networkLayerDataOrder Data order of the network, e.g. NCHW or HWC
 * @param numThreads Number of threads to use for inference
 * @return true if network was loaded and instantiated successfully, false otherwise (e.g.
 * model file does not exist or is invalid).
 */
bool claid::
		TensorFlowLiteNNInstance::setupModelFromAssets(
				AAssetManager* const assetManager, const std::string& modelFile, LayerDataOrder networkLayerDataOrder, const int numThreads)
{
	this->networkLayerDataOrder = networkLayerDataOrder;

    // Workaround for testing.
    //return this->setupModelFromFile();
    claid::Logger::logInfo("Using asset manager to load model file.");
    if(assetManager == NULL)
    {
        claid::Logger::logInfo("Reference to AssetManager is invalid (NULL).");
        return false;
    }



    claid::Logger::logInfo("Filename %s", modelFile.c_str());

    AAsset* asset =
            AAssetManager_open(assetManager, modelFile.c_str(), AASSET_MODE_BUFFER);

    if(asset == NULL)
    {
        claid::Logger::logInfo("Asset is NULL, probably the asset file that shall"
                                                  "be loaded was not packed into the apk.");
        return false;
    }
    off_t start;
    off_t length;
    const int fd = AAsset_openFileDescriptor(asset, &start, &length);


    // It may be compressed, in which case we have to uncompress
    // it to memory first.
    claid::Logger::logInfo("Opening asset %s rom disk with copy.",
                                              modelFile.c_str());
    off_t  dataSize = AAsset_getLength(asset);
    const void* const memory = AAsset_getBuffer(asset);
    const char* const memChar = (const char*) memory;
    this->flatBuffersBuffer = new char[dataSize];
    claid::Logger::logInfo("Copying assets buffer to flatbuffers buffer, size: %d", dataSize);
    for(int i = 0; i < dataSize; i++)
    {
        this->flatBuffersBuffer[i] = memChar[i];
    }


    this->model = tflite::FlatBufferModel::BuildFromBuffer(this->flatBuffersBuffer, dataSize);
    claid::Logger::logInfo("Loaded");
    AAsset_close(asset);
	if (this->model)
	{
		claid::Logger::logInfo("Setting up interpreter");
		this->model->error_reporter();

		tflite::InterpreterBuilder(*model, this->resolver)
			(&this->tfLiteInterpreter);

		if (this->tfLiteInterpreter)
		{
			claid::Logger::logInfo("Interpreter setup");
			this->tfLiteInterpreter->SetNumThreads(numThreads);
		//	this->tfLiteInterpreter->UseNNAPI(this->instanceSettings.useNNAPI);

			#if __TFNN_USE_GPU == 1
			if(this->instanceSettings.useGPU)
			{
				claid::Logger::logInfo("Trying to ModifyGraphWithDelegate.");
				this->gpuDelegate = TfLiteGpuDelegateCreate(nullptr);  // default config
				if (this->tfLiteInterpreter->ModifyGraphWithDelegate(this->gpuDelegate) != kTfLiteOk)
				{
					claid::Logger::logInfo("Failed to ModifyGraphWithDelegate, cannot use GPU.");
					this->lastError = TensorFlowLiteNNInstance::ErrorType::
							ERROR_MODIFY_GRAPH_WITH_DELEGATE_GPU;
					return false;
				}
				else
				{
					claid::Logger::logInfo("GPU support enabled!");
				}
				
			}
			#endif

			if (this->tfLiteInterpreter->AllocateTensors() == kTfLiteOk)
			{
				claid::Logger::logInfo("Tensors allocated.");
				claid::Logger::logInfo("tensors size: %d", this->tfLiteInterpreter->tensors_size());
				claid::Logger::logInfo("nodes size: %d", this->tfLiteInterpreter->nodes_size());
				claid::Logger::logInfo("inputs: %d", this->tfLiteInterpreter->inputs().size());
				claid::Logger::logInfo("input(0) name: %s", this->tfLiteInterpreter->GetInputName(0));
				return true;
			}
			else
			{
				claid::Logger::logInfo("failed to allocate tensors");
				this->lastError = TensorFlowLiteNNInstance::ErrorType::
						ERROR_NN_FAILED_TO_ALLOCATE_TENSORS;
				return false;
			}

		}
		else
		{
			claid::Logger::logInfo("Set up interpreter failed");
			this->lastError = TensorFlowLiteNNInstance::ErrorType::
					ERROR_FAILED_TO_CONSTRUCT_INTERPRETER;
			return false;
		}
	}
	else
	{
		this->lastError = TensorFlowLiteNNInstance::ErrorType::
				ERROR_COULD_NOT_MMAP_MODEL;
		return false;
	}

}
#endif


/**
 * Loads/copies input data into the input tensor with the given id.
 * 
 * @param inputID Input layer ID.
 * @param data Data to copy into input tensor.
 * @param numBytes Number of bytes to copy-
 * @return true if data was copied successfully, false otherwise (e.g.
 * if input layer id invalid oder number of bytes bigger than required by the layer).
 */
bool claid::TensorFlowLiteNNInstance::applyInput(const size_t inputID, const void* data, const size_t numBytes)
{

	// get input dimension from the input tensor metadata
	// assuming one input only

	int input = this->tfLiteInterpreter->inputs()[inputID];
	TfLiteTensor* inputTensor = this->tfLiteInterpreter->tensor(input);

	TfLiteIntArray* dims = inputTensor->dims;

	size_t expectedNumBytes;
	if(!this->getLayerDataTypeNumBytesFromTensor(inputTensor, expectedNumBytes))
	{
		this->lastError = TensorFlowLiteNNInstance::ErrorType::ERROR_NN_UNSUPPORTED_DATA_TYPE;
		return false;
	}

	for(int i = 0; i < dims->size; i++)
	{
		expectedNumBytes *= dims->data[i];
	}

	if(numBytes != expectedNumBytes)
	{
		this->lastError = TensorFlowLiteNNInstance::ErrorType::ERROR_APPLY_INPUT_BYTE_MISMATCH;
		return false;
	}

	void* inputArray = inputTensor->data.data;
	memcpy(inputArray, data, numBytes);

	return true;
}

/**
 * @brief Loads/copies input data into the input tensor descriped by the layerData.
 * 
 * @param layerData LayerData object holding data for a layer as well as a description of the 
 * target layer (layer name, dimension etc.).
 * @return true if data was copied successfully, false otherwise (e.g. invalid layer description,
 * input data containing mory bytes than required, etc.).
 */
bool claid::TensorFlowLiteNNInstance::applyInput(const LayerData& layerData)
{
	int id = this->getInputIDFromLayerName(layerData.layer_name());
	if(id == -1)
	{
		this->lastError = TensorFlowLiteNNInstance::ErrorType::ERROR_CANNOT_FIND_INPUT_LAYER_ID_BY_NAME;
		return false;
	}
	return this->applyInput(id, layerData.data().data(), getLayerDataNumBytes(layerData));
}


/**
 * Runs model inference, calculating the output of the neural network 
 * from the previously applied input data.
 * 
 * @return true if inference was successfull, false otherwise.
 */
bool claid::TensorFlowLiteNNInstance::runInference()
{
	if (this->tfLiteInterpreter->Invoke() == kTfLiteOk)
	{
		return true;
	}
	else
	{
		 this->lastError = TensorFlowLiteNNInstance::ErrorType::ERROR_NN_INVOKE_FAILED;
		 return false;
	}
}

/**
 * Copies data from the output layer(s) into a vector of LayerData objects.
 * An object is inserted into the vector for each output layer, containing the
 * data of the layer as well as a description of the layer (name, dimension, etc.).
 * 
 * @param layerDataVector Output layer data vector. 
 * @return true if data was copied successfully, false otherwise.
 */
bool claid::TensorFlowLiteNNInstance::getOutputLayerData(LayerDataVector& layerDataVector)
{
	layerDataVector.Clear();

	size_t outputLayers = this->tfLiteInterpreter->outputs().size();

	for(size_t i = 0; i < outputLayers; i++)
	{
		const int tfliteIDOfOutputLayer = this->tfLiteInterpreter->outputs()[i];
		TfLiteTensor* outputTensor = this->tfLiteInterpreter->tensor(tfliteIDOfOutputLayer);

		LayerDataType layerDataType;

		if(!this->layerDataTypeFromTensor(outputTensor, layerDataType))
		{
			this->lastError = TensorFlowLiteNNInstance::ErrorType::ERROR_NN_UNSUPPORTED_OUTPUT_TYPE;
			return false;
		}

		std::vector<int64_t> dimensions;
		this->getDimensionVectorFromTensor(outputTensor, dimensions);
		
		size_t numBytes;
		if(!this->getLayerDataTypeNumBytesFromTensor(outputTensor, numBytes))
		{
			this->lastError = TensorFlowLiteNNInstance::ErrorType::ERROR_NN_UNSUPPORTED_DATA_TYPE;
			return false;
		}

		for(size_t i = 0; i < dimensions.size(); i++)
		{
			numBytes *= dimensions[i];
		}
		LayerData* layerData = layerDataVector.add_layers();
		LayerDimension* layerDimensionPtr = layerData->mutable_layer_dimension();
		LayerDimension& layerDimension = *layerDimensionPtr;

		switch(dimensions.size())
		{
			case 4:
			{
				layerDimension.set_numbatches(dimensions[0]);
				layerDimension.set_height(dimensions[1]);
				layerDimension.set_width(dimensions[2]);
				layerDimension.set_channels(dimensions[3]);
			}
			break;
			case 3:
			{
				layerDimension.set_numbatches(1);
				layerDimension.set_height(dimensions[0]);
				layerDimension.set_width(dimensions[1]);
				layerDimension.set_channels(dimensions[2]);
			}
			break;
			case 2:
			{
				layerDimension.set_numbatches(1);
				layerDimension.set_height(1);
				layerDimension.set_width(dimensions[0]);
				layerDimension.set_channels(dimensions[1]);
			}
			break;
			case 1:
			{
				layerDimension.set_numbatches(1);
				layerDimension.set_height(1);
				layerDimension.set_width(1);
				layerDimension.set_channels(dimensions[0]);
			}
			break;
		}

		const std::string layerName = this->tfLiteInterpreter->GetOutputName(i);



		void* data = outputTensor->data.f;
		// LayerDimension is set via mutable_layer_dimension above.
		// layerData->set_layer_dimension(layerDimension);
		layerData->set_layer_name(layerName);
		layerData->set_data_order(this->networkLayerDataOrder);
		layerData->set_data(data, numBytes);
		layerData->set_data_type(layerDataType);

 		
	}
	return true;
}


/**
 * Returns the error code of the last occured error.
 * 
 * @return claid::TensorFlowLiteNNInstance::ErrorType Error code of last occured error.
 */
claid::TensorFlowLiteNNInstance::ErrorType
claid::TensorFlowLiteNNInstance::getLastError() const
{
	return this->lastError;
}

/**
 * Returns the error code of the last occured error as (human-readable) string.
 * E.g.: "ERROR_INVALID_MODEL_FILE"
 * @return std::string Error code as string.
 */
std::string claid::TensorFlowLiteNNInstance::getLastErrorString() const
{
	std::vector<std::string> names = {
	"ERROR_NONE",
	"ERROR_INVALID_MODEL_FILE",
	"ERROR_COULD_NOT_MMAP_MODEL",
	"ERROR_FAILED_TO_CONSTRUCT_INTERPRETER",
	"ERROR_NN_UNSUPPORTED_INPUT_TYPE",
	"ERROR_NN_UNSUPPORTED_OUTPUT_TYPE",
	"ERROR_NN_UNSUPPORTED_DATA_TYPE",
	"ERROR_NN_FAILED_TO_ALLOCATE_TENSORS",
	"ERROR_NN_INVOKE_FAILED",
	"ERROR_APPLY_INPUT_BYTE_MISMATCH",
	"ERROR_CANNOT_FIND_INPUT_LAYER_ID_BY_NAME",
	#if __TFNN_USE_GPU == 1
	"ERROR_MODIFY_GRAPH_WITH_DELEGATE_GPU",
	#endif
	"ERROR_TFNN_CLASSIFIER_INVALID_LABELS_FILE",

	#ifdef __ANDROID__
	// ====== Android specific Errors ==== //

	// If a model file could not be loaded from assets.
	"ERROR_LOAD_MODEL_FROM_ASSETS_FAILED",
	#endif
	};

	return names[static_cast<int>(this->lastError)];
}


/**
 * Checks whether a file with the given path exists. Should be platform independent.
 * 
 * @param name Path to a file.
 * @return true if file exists, false otherwise.
 */
bool claid::TensorFlowLiteNNInstance::fileExists(const std::string& name)
{
	struct stat buffer;
	return (stat (name.c_str(), &buffer) == 0);
}


/**
 * Determines the data type of a tensor (e.g. float, int, ...)
 * 
 * @param tensor Pointer to TfLiteTensor object.
 * @param layerDataType Output layer data type.
 * @return true TfLiteTensor is valid and it's data type is supported, false otherwise.
 */
bool claid::TensorFlowLiteNNInstance::layerDataTypeFromTensor(TfLiteTensor* tensor, LayerDataType& layerDataType)
{
	switch(tensor->type)
	{
		case TfLiteType::kTfLiteNoType:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteFloat32:
		{
			layerDataType = LayerDataType::FLOAT32;
			return true;
		}
		break;
		case TfLiteType::kTfLiteInt32:
		{
			layerDataType = LayerDataType::INT32;
			return true;
		}
		break;
		case TfLiteType::kTfLiteUInt32:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteInt64:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteString:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteBool:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteInt16:
		{
			layerDataType = LayerDataType::INT16;
			return true;
		}
		break;
		case TfLiteType::kTfLiteComplex64:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteInt8:
		{
			layerDataType = LayerDataType::INT8;
			return true;
		}
		break;
		case TfLiteType::kTfLiteUInt8:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteFloat16:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteFloat64:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteComplex128:
		{
			// Unsupported
			return false;
		}
		break;
		default:
		{
			return false;
		}
		break;
	}
}

/**
 * Returns the number of bytes needed by the data type of a tensor..
 * E.g. float32 requires 4 bytes, int16 2 and so on.
 * 
 * @param tensor Pointer to TfLiteTensor object.
 * @param numBytes Output num bytes.
 * @return true TfLiteTensor is valid and it's data type is supported, false otherwise.
 */
bool claid::TensorFlowLiteNNInstance::getLayerDataTypeNumBytesFromTensor(TfLiteTensor* tensor, size_t& numBytes)
{
	switch(tensor->type)
	{
		case TfLiteType::kTfLiteNoType:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteFloat32:
		{
			numBytes = sizeof(float);
			return true;
		}
		break;
		case TfLiteType::kTfLiteInt32:
		{
			numBytes = sizeof(int32_t);
			return true;
		}
		break;
		case TfLiteType::kTfLiteUInt32:
		{
			numBytes = sizeof(uint32_t);
			return true;
		}
		break;
		case TfLiteType::kTfLiteInt64:
		{
			numBytes = sizeof(int64_t);
			return true;
		}
		break;
		case TfLiteType::kTfLiteString:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteBool:
		{
			numBytes = sizeof(bool);
			return true;
		}
		break;
		case TfLiteType::kTfLiteInt16:
		{
			numBytes = sizeof(int16_t);
			return true;
		}
		break;
		case TfLiteType::kTfLiteComplex64:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteInt8:
		{
			numBytes = sizeof(int8_t);
			return true;
		}
		break;
		case TfLiteType::kTfLiteFloat16:
		{
			// Unsupported
			return false;
		}
		break;
		case TfLiteType::kTfLiteFloat64:
		{
			numBytes = sizeof(double);
			return true;
		}
		break;
		case TfLiteType::kTfLiteComplex128:
		{
			// Unsupported
			return false;
		}
		break;
		default:
		{
			return false;
		}
		break;
	}
}

/**
 * Retrieves the dimensions of a tensor object.
 * E.g. (N, C, H, W) = (1, 3, 320, 160).
 * 
 * @param tensor Pointer to TfLiteTensor object.
 * @param dimensions Output dimensions of tensor.
 */
void claid::TensorFlowLiteNNInstance::getDimensionVectorFromTensor(TfLiteTensor* tensor, std::vector<int64_t>& dimensions)
{
	dimensions.clear();
	TfLiteIntArray* dims = tensor->dims;

	size_t numBytes = 1;

	for(int i = 0; i < dims->size; i++)
	{
		numBytes *= dims->data[i];
		dimensions.push_back(dims->data[i]);
	}
}

/**
 * Determines the input layer id of the layer with the given name.
 * 
 * @param layerName Name of an input layer.
 * @return int ID of the input layer.
 */
int claid::TensorFlowLiteNNInstance::getInputIDFromLayerName(const std::string& layerName)
{
	const int numInputLayers = this->tfLiteInterpreter->inputs().size();
	for(int i = 0; i < numInputLayers; i++)
	{
		if(this->tfLiteInterpreter->GetInputName(i) == layerName)
		{
			return i;
		}
	}
	return -1;
}

/**
 * Determines the output layer id of the layer with the given name.
 * 
 * @param layerName Name of an output layer.
 * @return int ID of the output layer.
 */
int claid::TensorFlowLiteNNInstance::getOutputIDFromLayerName(const std::string& layerName)
{
	const int numOutputLayers = this->tfLiteInterpreter->outputs().size();
	for(int i = 0; i < numOutputLayers; i++)
	{
		if(this->tfLiteInterpreter->GetOutputName(i) == layerName)
		{
			return i;
		}
	}
	return -1;
}

/**
 * Determines the number of bytes the input layer with the given ID
 * requires, i.e. it's dimension * number of bytes of it's datatype.
 * 
 * @param inputID Input layer ID.
 * @param numBytes Output calculated number of bytes of the input layer.
 * @return true TfLiteTensor is valid and it's data type is supported, false otherwise.
 */
bool claid::TensorFlowLiteNNInstance::getInputLayerNumBytes(const size_t inputID, size_t& numBytes)
{
	int input = this->tfLiteInterpreter->inputs()[inputID];
	TfLiteTensor* inputTensor = this->tfLiteInterpreter->tensor(input);

	TfLiteIntArray* dims = inputTensor->dims;

	if(!this->getLayerDataTypeNumBytesFromTensor(inputTensor, numBytes))
	{
		this->lastError = TensorFlowLiteNNInstance::ErrorType::ERROR_NN_UNSUPPORTED_DATA_TYPE;
		return false;
	}

	for(int i = 0; i < dims->size; i++)
	{
		numBytes *= dims->data[i];
	}

	return true;

}

/**
 * Determines the number of bytes the output layer with the given ID
 * requires, i.e. it's dimension * number of bytes of it's datatype.
 * 
 * @param inputID Output layer ID.
 * @param numBytes Output calculated number of bytes of the input layer.
 * @return true TfLiteTensor is valid and it's data type is supported, false otherwise.
 */
bool claid::TensorFlowLiteNNInstance::getOutputLayerNumBytes(const size_t outputID, size_t& numBytes)
{
	int output = this->tfLiteInterpreter->outputs()[outputID];
	TfLiteTensor* outputTensor = this->tfLiteInterpreter->tensor(output);

	TfLiteIntArray* dims = outputTensor->dims;

	if(!this->getLayerDataTypeNumBytesFromTensor(outputTensor, numBytes))
	{
		this->lastError = TensorFlowLiteNNInstance::ErrorType::ERROR_NN_UNSUPPORTED_DATA_TYPE;
		return false;
	}

	for(int i = 0; i < dims->size; i++)
	{
		numBytes *= dims->data[i];
	}

	return true;

}

/**
 * Returns the number of bytes available in a passed layerData object.
 * 
 * @param LayerData layerData
 * @return size_t number of bytes
 */
size_t claid::TensorFlowLiteNNInstance::getLayerDataNumBytes(const LayerData& layerData)
{
	return layerData.data().size();	
}

/**
 * Destructor of TensorFlowLiteNNInstance.
 * Releases allocated ressources and buffers
 * as well as the tensorflow lite interpreter.
 * 
 */
claid::TensorFlowLiteNNInstance::~TensorFlowLiteNNInstance()
{
    #ifdef __ANDROID__
        delete[] this->flatBuffersBuffer;
    #endif

	#if __TFNN_USE_GPU == 1
		if(this->instanceSettings.useGPU)
		{
			TfLiteGpuDelegateDelete(this->gpuDelegate);
		}
	#endif
	
	this->tfLiteInterpreter.release();
}

