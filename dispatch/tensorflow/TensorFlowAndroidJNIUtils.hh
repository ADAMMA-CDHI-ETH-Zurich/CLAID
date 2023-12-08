#pragma once

#include <jni.h>
#include <string>
#include <vector>
#include <string>


#include "tensorflow/lite/interpreter.h"

#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tensorflow/lite/string_util.h"


class AAssetManager;

/**
 * Utility class that allows to load tensorflow lite models
 * from android assets.
**/
namespace claidp
{
	class TensorflowAndroidJNIUtils
	{
		public:
		bool static LoadModelFileFromAsset(AAssetManager* const asset_manager,
							std::string filename,
		                    tflite::FlatBufferModel* model);
	};

}

