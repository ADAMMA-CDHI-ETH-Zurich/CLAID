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
import 'package:claid/Logger/Logger.dart';
import 'package:claid/generated/claidservice.pb.dart';

class RemoteFunctionRunnableResult<T>
{
    T? _returnValue;
    RemoteFunctionStatus _status;

    RemoteFunctionRunnableResult(this._returnValue, this._status)
    {

    }


    static RemoteFunctionRunnableResult<A> makeSuccessfulResult<A>(A returnValue)
    {
        return RemoteFunctionRunnableResult<A>(returnValue, RemoteFunctionStatus.STATUS_OK);
    }

    static RemoteFunctionRunnableResult<A> makeFailedResult<A>(RemoteFunctionStatus status)
    {
        return RemoteFunctionRunnableResult<A>(null, status);
    }

    RemoteFunctionStatus getStatus()
    {
        return this._status;
    }

    T? getReturnValue()
    {
        return this._returnValue;
    }
}