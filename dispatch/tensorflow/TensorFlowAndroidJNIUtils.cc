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
    claid::Logger::printfln("Using asset manager to load model file.");
   if(asset_manager == NULL)
   {
       claid::Logger::printfln("Reference to AssetManager is invalid (NULL).");
       return false;
   }

  std::string asset_filename = ASSET_PREFIX;
   asset_filename.append(filename);

   claid::Logger::printfln("Filename %s %s", filename.c_str(), asset_filename.c_str());

  AAsset* asset =
      AAssetManager_open(asset_manager, filename.c_str(), AASSET_MODE_STREAMING);

  if(asset == NULL)
  {
    claid::Logger::printfln("Asset is NULL, probably the asset file that shall"
                                                      "be loaded was not packed into the apk.");
    return false;
  }
  off_t start;
  off_t length;
  const int fd = AAsset_openFileDescriptor(asset, &start, &length);


    // It may be compressed, in which case we have to uncompress
    // it to memory first.
    claid::Logger::printfln("Opening asset %s rom disk with copy.",
                                              asset_filename.c_str());
    off_t  dataSize = AAsset_getLength(asset);
    const void* const memory = AAsset_getBuffer(asset);

    claid::Logger::printfln("Building 1");
    model->BuildFromBuffer((const char*) memory, dataSize);
    claid::Logger::printfln("Building 2");
    AAsset_close(asset);
    return true;
}

#endif