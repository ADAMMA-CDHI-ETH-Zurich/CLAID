//
//  TestAppDelegate.swift
//  SpeziCLAID
//
//  Created by Patrick Langer on 10.03.2025.
//

@testable import Spezi
import Foundation
import CLAID
import UIKit

final actor MyModule : CLAIDModule {
 
    @Dependency
    var claidRuntime = CLAIDRuntime()
    let moduleHandle: ModuleHandle
    
    init(
        id: String = "MyModule"
    ) {
        moduleHandle = Self.makeDefaultHandle(id: id)
    }

    @Sendable
    func periodic() async {
        print("Periodic function")
    }
    
    func run() async throws {
        print("Run called!")
        await registerPeriodicFunction(
            name: "TestFunction",
            interval: Duration.milliseconds(100),
            function: self.periodic
        )
    }
    
    func terminate() async {
        
    }
}




class TestAppDelegate : SpeziAppDelegate {
    override var configuration: Configuration  {
        Configuration {
            MyModule()
            
        }
    }
}
