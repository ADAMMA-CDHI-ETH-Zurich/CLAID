import Foundation

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
