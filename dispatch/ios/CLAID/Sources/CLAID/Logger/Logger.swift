//
//  Logger.swift
//  CLAID
//
//  Created by Patrick Langer on 19.02.2025.
//


import Foundation

public class Logger {
    
    static let logSeverityLevelToPrint: Claidservice_LogMessageSeverityLevel = .info
    
    static func log(_ level: Claidservice_LogMessageSeverityLevel, _ message: Any, logMessageEntityType: Claidservice_LogMessageEntityType = .middleware, logMessageEntity: String = "SWIFT_RUNTIME") {
        let logLevel: String = "\(level)"
        
        let output = "[\(getTimeString()) | \(logLevel.uppercased()) | CLAID Swift] \(message)\n"
        
        if let messageString = message as? String {
            if level.rawValue >= logSeverityLevelToPrint.rawValue {
                if level == .error || level == .fatal {
                    fputs(output, stderr)
                } else {
                    print(output)
                }
            }
        }

    }
    
    static func logDebug(_ message: Any) {
        log(.debugVerbose, message)
    }
    
    static func logInfo(_ message: Any) {
        log(.info, message)
    }
    
    static func logWarning(_ message: Any) {
        log(.warning, message)
    }
    
    static func logError(_ message: Any) {
        log(.error, message)
    }
    
    static func logFatal(_ message: Any) {
        log(.fatal, message)
    }
    
    static func getTimeString() -> String {
        let formatter = DateFormatter()
        formatter.dateFormat = "dd.MM.yyyy - HH:mm:ss"
        return formatter.string(from: Date())
    }
    
    static func logThrowFatal(_ message: Any) throws {
        logFatal(message)
        throw NSError(domain: "Logger", code: 1, userInfo: [NSLocalizedDescriptionKey: "\(message)"])
    }
}
