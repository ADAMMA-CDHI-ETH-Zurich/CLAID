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



import 'dart:async';
import 'dart:collection';

import 'package:claid/dispatcher.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/generated/google/protobuf/struct.pb.dart';

import 'package:claid/mocks.dart';
import 'package:claid/module/properties.dart';

import 'package:claid/module/module.dart';
import 'package:claid/module/module_manager.dart';

import 'package:claid/module/type_mapping.dart';

enum Lifecycle {
  unknown,
  created,
  initializing,
  initialized,
  running,
  terminated
}

class ModuleState {
  final Module instance;
  Struct props;
  Lifecycle lifecycle;
  ModuleState(this.instance, this.props, this.lifecycle);
}

typedef ReceiverFunc = void Function(DataPackage);

class Scheduler {
  final _periodic = <String, Timer>{};
  final _scheduled = <String, Timer>{};

  void registerPeriodicFunction(String modId, String regName, Duration period,
      RegisteredCallback callback) {
    String regId = combineIds(modId, regName);
    if (_periodic.containsKey(regId) || regName.isEmpty) {
      throw ArgumentError(
          'Name for periodic function "$regId" already in use or empty');
    }
    _periodic[regId] = Timer.periodic(period, (timer) => callback());
  }

  void unregisterPeriodicFunction(String modId, String regName) {
    String regId = combineIds(modId, regName);
    if (!_periodic.containsKey(regId)) {
      throw ArgumentError('Periodic function "$regId" not registered');
    }
    _periodic.remove(regId)?.cancel();
  }

  void registerScheduledFunction(String modId, String regName,
      DateTime targetTime, RegisteredCallback callback) {
    String regId = combineIds(modId, regName);

    // Make sure the function has not been scheduled before.
    if (_scheduled.containsKey(regId) || regName.isEmpty) {
      throw ArgumentError(
          'Name for scheduled function "$regId" already in use or empty');
    }

    final now = DateTime.now();
    if (!targetTime.isAfter(now)) {
      throw ArgumentError('Scheduled event "$regId" must be in the future');
    }

    _scheduled[regId] = Timer(targetTime.difference(now), () async {
      _scheduled.remove(regId);
      await callback();
    });
  }

  void unregisterScheduledFunction(String modId, String regName) {
    String regId = combineIds(modId, regName);
    if (!_scheduled.containsKey(regId)) {
      throw ArgumentError('S function "$regId" not registered');
    }
    _scheduled.remove(regId)?.cancel();
  }
}

String combineIds(String modId, String secondary) => '$modId:$secondary';
