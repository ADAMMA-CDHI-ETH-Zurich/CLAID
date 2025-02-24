//
//  Logger.swift
//  CLAID
//
//  Created by Patrick Langer on 19.02.2025.
//


import Foundation

public class Logger {
    
    static let logSeverityLevelToPrint: Claidservice_LogMessageSeverityLevel = .info
    
    static func log(_ level: Claidservice_LogMessageSeverityLevel, _ message: String, logMessageEntityType: Claidservice_LogMessageEntityType = .middleware, logMessageEntity: String = "SWIFT_RUNTIME") {
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
    
    static func logDebug(_ message: String) {
        log(.debugVerbose, message)
    }
    
    static func logInfo(_ message: String) {
        log(.info, message)
    }
    
    static func logWarning(_ message: String) {
        log(.warning, message)
    }
    
    static func logError(_ message: String) {
        log(.error, message)
    }
    
    static func logFatal(_ message: String) {
        log(.fatal, message)
        fatalError(message)
    }
    
    static func getTimeString() -> String {
        let formatter = DateFormatter()
        formatter.dateFormat = "dd.MM.yyyy - HH:mm:ss"
        return formatter.string(from: Date())
    }
    
    static func logThrowFatal(_ message: String) throws {
        logFatal(message)
        throw CLAIDError("\(message)")
    }
}
