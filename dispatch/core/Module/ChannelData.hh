/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
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

#pragma once

#include "dispatch/core/Utilities/Time.hh"

namespace claid{
    
    template<typename T>
    struct ChannelData
    {
    private:
            std::shared_ptr<T> data;
            Time timestamp;
            std::string userId;

            bool valid = false;
    public:

        ChannelData() : valid(false)
        {

        }

        ChannelData(std::shared_ptr<T> data, 
            const Time& timestamp, const std::string& userId) : data(data), timestamp(timestamp), userId(userId), valid(true)
        {

        }

        static ChannelData fromCopy(const T& data, const Time& timestamp, const std::string& userId)
        {
            std::shared_ptr<T> sharedDataCopy = std::make_shared<T>(data);

            return ChannelData(sharedDataCopy, timestamp, userId);
        }


        const T& getData()
        {
            return *this->data.get();
        }

        std::shared_ptr<const T> getDataAsPtr()
        {
            return this->data;
        }

        const Time& getTimestamp()
        {
            return this->timestamp;
        }

        const std::string& getUserId()
        {
            return this->userId;
        }

        bool isValid() const
        {
            return this->valid;
        }

    };
}