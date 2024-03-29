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

#include "CLAIDWrapper.hpp"
#import <Foundation/Foundation.h>
#include "dispatch/core/CLAID.hh"
bool CLAIDWrapper::start(const std::string& configPath, const std::string& hostId, const std::string& userId, const std::string& deviceId)
{
    if(claidInstance != nullptr)
    {
        return false;
    }

    claidInstance = new claid::CLAID();
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    std::string stdDocuments([documentsDirectory UTF8String]);

    thread = std::make_unique<std::thread>([=, this] {
        bool result = claidInstance->start("localhost:1337", configPath, hostId, userId, deviceId, stdDocuments);
    });
    return true;
}


