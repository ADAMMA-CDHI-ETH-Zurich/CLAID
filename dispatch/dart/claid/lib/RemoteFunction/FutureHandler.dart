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

import "dart:async";
import "RPCCompleter.dart";
import "FutureUniqueIdentifier.dart";
import "FuturesTable.dart";
import "AbstractRPCCompleter.dart";
// Why is this called FutureHandler and not RPCComplerHandler? 
// -> Check Java and C++ implementations. For Java and C++, we implemented custom Future classes.
// For Dart, we use the integrated Future data type.
class FutureHandler {
  FuturesTable openFutures = FuturesTable();

  RPCCompleter<T> registerNewFuture<T>(T returnDataTypeExample) 
  {
    FutureUniqueIdentifier uniqueIdentifier =
        FutureUniqueIdentifier.makeUniqueIdentifier();

    RPCCompleter<T> completer =
        RPCCompleter<T>(openFutures, uniqueIdentifier, returnDataTypeExample);

    print("Registering future");
    openFutures.addFuture(completer);

    return completer;
  }

  AbstractRPCCompleter? lookupFuture(FutureUniqueIdentifier identifier) 
  {
    openFutures.printFutures();
    return openFutures.lookupFuture(identifier);
  }
}

