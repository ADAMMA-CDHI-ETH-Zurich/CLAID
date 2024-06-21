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

import 'FutureUniqueIdentifier.dart';
import 'AbstractRPCCompleter.dart';

class FuturesTable 
{
    Map<FutureUniqueIdentifier, AbstractRPCCompleter> futures = {};

    void addFuture(AbstractRPCCompleter future)
    {
        futures[future.getUniqueIdentifier()] = future;
    }

    bool removeFuture(AbstractRPCCompleter future)
    {
        print("Remove future");
        FutureUniqueIdentifier futureIdentifier = future.getUniqueIdentifier();

        if(futures.containsKey(futureIdentifier))
        {
            AbstractRPCCompleter other = futures[futureIdentifier]!;
            if(other == future)
            {
                futures.remove(futureIdentifier);
                return true;
            }
        }
        return false;
    }

    AbstractRPCCompleter? lookupFuture(FutureUniqueIdentifier uniqueIdentifier)
    {
        AbstractRPCCompleter? future = null;
        if(this.futures.containsKey(uniqueIdentifier))
        {
            future = this.futures[uniqueIdentifier];
        }
        return future;
    }

    void printFutures()
    {
        print(futures.toString());
    }
    
}
