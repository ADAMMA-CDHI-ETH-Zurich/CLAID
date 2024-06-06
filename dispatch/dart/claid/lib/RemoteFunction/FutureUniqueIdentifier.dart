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


import 'dart:async';
class FutureUniqueIdentifier 
{
    static int _currentId = 0;

    String identifier;

    static FutureUniqueIdentifier makeUniqueIdentifier()
    {
        DateTime now = DateTime.now();
        int milliseconds = now.millisecondsSinceEpoch;


        String identifier = "CLAID_DART_" + FutureUniqueIdentifier._currentId.toString() + "_" + milliseconds.toString();

        FutureUniqueIdentifier._currentId++;
        return new FutureUniqueIdentifier(identifier);
    } 



    FutureUniqueIdentifier(this.identifier)
    {
    }

    String toString()
    {
        return this.identifier;
    }

    bool equals(FutureUniqueIdentifier other)
    {
        return this.identifier == other.toString();
    }

    bool operator==(Object o) 
    {
        if (this == o)
            return true;

        if (o is! FutureUniqueIdentifier)
            return false;

        FutureUniqueIdentifier other = o as FutureUniqueIdentifier;
        return this.equals(other);
    }

    int get hashCode => this.identifier.hashCode; 
 
}
