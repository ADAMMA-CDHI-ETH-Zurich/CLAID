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
