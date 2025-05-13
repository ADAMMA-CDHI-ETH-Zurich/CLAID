//
//  TestAppDelegate.swift
//  SpeziCLAID
//
//  Created by Patrick Langer on 10.03.2025.
//


import Spezi
import Foundation
import protocol CLAID.Module
import class CLAID.ModuleHandle


actor MyModule : Module {
    var moduleHandle = CLAID.ModuleHandle()
    
    func run() async throws {
        print("Run called!")
    }
    
    func terminate() async {
        
    }
    
    
}

class TestAppDelegate: SpeziAppDelegate {
    override var configuration: Configuration {
        Configuration(standard: ExampleStandard()) {
            
            MyModule()
        }
    }
    

}
