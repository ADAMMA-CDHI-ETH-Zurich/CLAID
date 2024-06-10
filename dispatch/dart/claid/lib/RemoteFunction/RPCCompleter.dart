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

import 'AbstractRPCCompleter.dart';
import 'FuturesTable.dart';
import 'FutureUniqueIdentifier.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/module/type_mapping.dart';

import 'package:flutter_test/flutter_test.dart';

import 'dart:async';
import 'dart:io';

class RPCCompleter<T> extends AbstractRPCCompleter 
{
    T _returnTypeExample;

    Completer<T> _typedCompleter = Completer<T>();

    RPCCompleter(FuturesTable futuresTableInHandler, 
        FutureUniqueIdentifier uniqueIdentifier,
        this._returnTypeExample) : super(futuresTableInHandler, uniqueIdentifier) 
    {
      
    }

    Future<T?> getResponse()
    {
        Future<DataPackage?> response = super.getResponseUntyped();
        response.then((dataPackage)
        {
            T? result = null;
            
            if(dataPackage != null)
            {
                result = _getReturnData(dataPackage!);
            }
            _typedCompleter.complete(result);
        }).catchError((error){
            _typedCompleter.complete(null);
        });
        
        return _typedCompleter.future;
    }

    T? _getReturnData(DataPackage responsePackage)
    {
        if(responsePackage == null)
        {
            return null;
        }
        // For void functions.
        if(T == _getType<void>())
        {
            return null;
        }

        Mutator<T> mutator = TypeMapping().getMutator(this._returnTypeExample);

        T t = mutator.getter(responsePackage);

        return t;
    }
    Type _getType<T>() => T;
}