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

#pragma once
#include "FuturesTable.hh"
#include "Future.hh"

namespace claid 
{
    class FutureHandler 
    {
        private:
            FuturesTable openFutures;

        public:
        
            template<typename T>
            std::shared_ptr<Future<T>> registerNewFuture()
            {
                FutureUniqueIdentifier uniqueIdentifier = FutureUniqueIdentifier::makeUniqueIdentifier();
                std::shared_ptr<Future<T>> future = std::make_shared<Future<T>>(this->openFutures, uniqueIdentifier);

                this->openFutures.addFuture(std::static_pointer_cast<AbstractFuture>(future));

                return future;
            };

            std::shared_ptr<AbstractFuture> lookupFuture(FutureUniqueIdentifier identifier)
            {
                return openFutures.lookupFuture(identifier);
            }

    };
}

