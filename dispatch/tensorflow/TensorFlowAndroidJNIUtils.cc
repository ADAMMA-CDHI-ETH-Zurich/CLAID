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

#ifdef __ANDROID__



#include "TensorFlowAndroidJNIUtils.h"
#include "Logger.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <stdlib.h>

#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>



static const char* const ASSET_PREFIX = "file:///android_asset/";


bool claid::TensorflowAndroidJNIUtils::LoadModelFileFromAsset(AAssetManager* const asset_manager,
					std::string filename, tflite::FlatBufferModel* model)
{
    claid::Logger::logInfo("Using asset manager to load model file.");
   if(asset_manager == NULL)
   {
       claid::Logger::logInfo("Reference to AssetManager is invalid (NULL).");
       return false;
   }

  std::string asset_filename = ASSET_PREFIX;
   asset_filename.append(filename);

   claid::Logger::logInfo("Filename %s %s", filename.c_str(), asset_filename.c_str());

  AAsset* asset =
      AAssetManager_open(asset_manager, filename.c_str(), AASSET_MODE_STREAMING);

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
                                              asset_filename.c_str());
    off_t  dataSize = AAsset_getLength(asset);
    const void* const memory = AAsset_getBuffer(asset);

    claid::Logger::logInfo("Building 1");
    model->BuildFromBuffer((const char*) memory, dataSize);
    claid::Logger::logInfo("Building 2");
    AAsset_close(asset);
    return true;
}

#endif