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
import 'FuturesTable.dart';
import 'FutureUniqueIdentifier.dart';

import 'package:claid/generated/claidservice.pb.dart';


abstract class AbstractRPCCompleter 
{
    bool _finished = false;
    bool _successful = false;

    Completer<DataPackage?> _completer = Completer<DataPackage>();

    // The list used by the FutureHandler to manage its associated futures.
    // Note: FuturesTable is thread safe.
    FuturesTable futuresTableInHandler;

    FutureUniqueIdentifier uniqueIdentifier;

    
    AbstractRPCCompleter(this.futuresTableInHandler, this.uniqueIdentifier)
    {
  
    }

    Future<DataPackage?> getResponseUntyped()
    {
        return _completer.future;
    }
    

    void setResponse(DataPackage responsePackage) 
    {
        this._successful = true;

        if(!_completer.isCompleted)
        {
            _completer.complete(responsePackage);
        }

        this._finished = true;

        // This is thread safe, as FuturesList is thread safe
        this.futuresTableInHandler.removeFuture(this);
    }

    void setFailed()
    {
        this._successful = false;
        
        if(!_completer.isCompleted)
        {
            _completer.complete(null);
        }

        this._finished = true;
    }

    FutureUniqueIdentifier getUniqueIdentifier()
    {
        return this.uniqueIdentifier;
    }

    bool wasExecutedSuccessfully()
    {
        return this._successful && this._finished;
    }
}
