## 0.6.0
* First major release
* Added x86 support
* Added remote configuration and config upload
* Added various control messages for reconfiguration, pausing and unpausing Modules
* Added Power Profiles
* Added adaptive battery management
* Added more collectors 
* Added packaging system
* Reworked Dart API to be conform to Java, Python and C++ APIs
* Added RPCs, allowing Modules and non-Module entities to remotely call functions of other Modules or Runtimes across languages.
* Added more UI elements
* Added matching of CLAIDModuleViews to corresponding Modules, enabling to register views for certain Modules (even if written in another language)
* Added CLAIDView for simple setup of CLAID in flutter
* Removed need for PersistentModuleFactory for CLAID in Android services. 
* Added checks for battery exemptions and device management features
* Added CLAIDDeviceOwnerFeatures, allowing to control Wifi and Bluetooth from the background without user intervention, if App is registered as device owner.

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
