import Foundation
import CLAIDNative
/// Calls the C function `start_core(...)`.
/// - Parameters:
///   - socketPath: A socket path string.
///   - configFile: A path to a config file.
///   - hostID:     Host identifier.
///   - userID:     User identifier.
///   - deviceID:   Device identifier.
/// - Returns:      A raw pointer handle returned by the C function.
public func startCore(
    socketPath: String,
    configFile: String,
    hostID: String,
    userID: String,
    deviceID: String
) -> UnsafeMutableRawPointer? {
    // Convert each Swift string to a null-terminated C string
    return socketPath.withCString { socketCStr in
        configFile.withCString { configCStr in
            hostID.withCString { hostCStr in
                userID.withCString { userCStr in
                    deviceID.withCString { deviceCStr in
                        // Call the C function directly
                        let handle = start_core(
                            socketCStr,
                            configCStr,
                            hostCStr,
                            userCStr,
                            deviceCStr
                        )
                        return handle
                    }
                }
            }
        }
    }
}


/// Initializes the core with an event tracker
func startCoreWithEventTracker(socketPath: String, configFile: String, hostID: String, userID: String, deviceID: String, commonDataPath: String) -> UnsafeMutableRawPointer?  {
    let handle = socketPath.withCString { socketCStr in
        configFile.withCString { configCStr in
            hostID.withCString { hostCStr in
                userID.withCString { userCStr in
                    deviceID.withCString { deviceCStr in
                        commonDataPath.withCString { commonDataCStr in
                            return start_core_with_event_tracker(
                                socketCStr,
                                configCStr,
                                hostCStr,
                                userCStr,
                                deviceCStr,
                                commonDataCStr
                            )
                        }
                    }
                }
            }
        }
    }
    
    return handle
}

/// Attaches a C++ runtime to the existing handle
func attachCppRuntime(handle: UnsafeMutableRawPointer?) throws -> UnsafeMutableRawPointer?{
    guard let currentHandle = handle else {
        throw CLAIDError("Error: Core is not initialized")
    }
    let c_runtime_handle = attach_cpp_runtime(currentHandle)
    
    if(c_runtime_handle == nil) {
        throw CLAIDError("Failed to start C++ runtime. Returned C runtime handle is null.")
    }
    
    return c_runtime_handle
}

/// Shuts down the core and cleans up resources
func shutdown(handle: UnsafeMutableRawPointer?) {
    guard let currentHandle = handle else {
        print("Error: Core is not initialized")
        return
    }
    shutdown_core(currentHandle)
}
