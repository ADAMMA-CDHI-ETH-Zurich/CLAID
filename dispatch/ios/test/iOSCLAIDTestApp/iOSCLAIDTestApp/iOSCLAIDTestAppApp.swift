import SwiftUI
import CLAID

@main
struct iOSCLAIDTestAppApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}

struct ContentView: View {
  

    var body: some View {
        Text("Hello, world!")
            .onAppear {
                Task { @MainActor in
                    await asyncInit()
                }
            }
    }
    
    func getTestConfigPath() -> String? {
        // Get the path to the resource inside the Swift package
        if let fileURL = Bundle.main.url(forResource: "test_config", withExtension: "json") {
            return fileURL.path // Convert URL to a file path string
        }
        print("Test config not found!!")
        return nil
    }
    
    @MainActor
    func asyncInit() async {
        do {
            
            if let testConfigPath = getTestConfigPath(){
                
                let moduleFactory = ModuleFactory()
                try await moduleFactory.registerModule(TestModule.self)
                try await moduleFactory.registerModule(TestModule2.self)
                
                try await CLAID.start(
                    configFile: testConfigPath,
                    hostID: "test_host",
                    userID: "test_user",
                    deviceID: "test_device",
                    moduleFactory: moduleFactory
                )
            }
            else {
                print("Failed to get path to test config file.")
            }
       
            
            
            
        } catch {
            print("Error starting CLAID: \(error)")
        }
    }
}
