## 0.6.4
* Fixed bug in isConnectedToRemoteServer, leading to an invalid state if the connection to the remote server was established before all Modules were initialized
  * isConnectedToRemoteServer now directly uses a RemoteFunction to request the connection state from the middleware, instead of keeping track of the state for each Module
  * Added isConnectedToRemoteServer to Java API as well
* Fixed bug in setSyncingSchedule of DataSyncModule, leading to functions not being rescheduled based on a new Schedule
* Fixed onCLAIDStarted not being called sometimes, when starting CLAID in the foreground
* Improved error handling, changed some errors to fatals to ensure the user will notice it
* Added DeviceInfoGatherer, allowing to request charging state and battery level on mobile devices from other runtimes (currently only C++, only on Android)
* Added optional parameter to Futures, allowing them to timeout if a response was not received in time.

## 0.6.3
* Fixed bug in global scheduling in Java, that would cause some threads to not wake up when due.
* Added Schedule data type, allowing to easily specify schedules in terms of intervals, frequencies, exact times or time windows.
* Added registerFunctionBasedOnSchedule, allowing to easily register function calls based on a specified schedule.
  * Schedule can be specified as property, hence can be set from the config file, allowing to specify schedules very easily
  * Changed BatteryCollector to use Schedule
  * Changed DataSyncModule to use Schedule
* Added remote functions "start_sync" and "set_sync_schedule" to DataSyncModule, allowing other Modules to start the synchronization process and to change its schedule.
* Added support for for enabling TLS and mutual TLS (mTLS) encryption and authentication from configuration files.
  * Added Test for TLS
* Changed CLAID WakeLocks to leverage reference counting, so acquiring a wakelock X times and releasing it X times is safe and will not cause issues.
* Changed AccelerometerCollector and GyroscopeCollector to use WakeLocks.

## 0.6.2
* Fixed device view button not having the correct index when clicked on in the ModuleList.
* Added onShown() and onHidden() to CLAIDModuleView. Will be called whenever a view is selected from the CLAIDModuleList.
* Ensuring Modules are shown in sorted order in CLAIDModuleList.
* Improved DataSyncModule and DataReceiverModule; both now include better user handling.
* Added RemoteFunctionWith1Parameter, RemoteFunctionWith2Parameters, ... to Dart to provide a more convenient execute function.
* Added GlobalDeviceScheduler to have "perfect" scheduling across all Runtimes
  * All Runtimes can call middleware functions like acquireWakelock, releaseWakelock and scheduleDeviceWakeupAtTime to let the middleware know about their scheduling requirements.
  * The middleware then uses a platform specific GlobalDeviceScheduler, which uses platform specific code (e.g., on Android/WearOS and iOS) to acquire/release wakelocks and schedule device wakeups.
  * This should guarantee that whenever any runtime schedules a Runnable by any means, that the device will be awake at the due date/time and keeps awake while the scheduled function is being executed.
  * Providing global device scheduling functions to all supported Runtimes; starting with Java and C++ for now.

## 0.6.1
* First major release of CLAID!
* Added x86 support
* Added remote configuration and config upload
* Added various control messages for reconfiguration, pausing and unpausing Modules
* Added Power Profiles
* Added adaptive battery management
* Added more collectors 
* Added packaging system
* Reworked Dart API to conform to Java, Python, and C++ APIs
* Added RPCs, allowing Modules and non-module entities to call functions of other Modules or Runtimes across languages remotely.
* Added more UI elements
* Beautified CLAID Module widgets
* Added matching of CLAIDModuleViews to corresponding Modules, enabling to register views for certain Modules (even if written in another language)
* Added CLAIDView for simple visualization of all currently loaded CLAID Modules in Flutter
* Removed the need for PersistentModuleFactory for CLAID in Android services. 
* Added checks for battery exemptions and device management features
* Added CLAIDDeviceOwnerFeatures, allowing to control Wifi and Bluetooth from the background without user intervention, if the App is registered as a device owner.
* Added automatic timeout at startup, enabling the middleware to assert that all Modules specified in the config are loaded. Timeout can be set from the config file.
  

## 0.0.5
* Added ABI filters, because this flutter package currently does not support x86 and x86_64 devices.

## 0.0.4
* Updated proto sources




## 0.0.3 
* Updating Readme




## 0.0.2
* Updating LICENSE
* Updating README
* Updating pubspec


## 0.0.1

* CLAID Middleware implementation 
    * gRPC Server and Runtime Clients
    * Configuration loading and checking
    * Tests
* Dart Runtime
    * Modules
    * Scheduling (periodic and scheduled functions)
    * Common data types (bool, int, string, array, map)
    * Protobuf data types
    * Tests
* Java Runtime
    * Modules
    * Scheduling (periodic and scheduled functions)
    * Common data types (bool, int, string, array, map)
    * Protobuf data types
* Support for Android, Linux and macOS
    * Android libs for armeabi-v7a, arm64-v8a, x86 and x86_64 
