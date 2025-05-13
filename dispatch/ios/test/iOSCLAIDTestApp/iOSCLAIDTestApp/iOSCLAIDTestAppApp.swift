import Spezi
import SwiftUI

@main
struct ExampleApp: App {
    @ApplicationDelegateAdaptor(TestAppDelegate.self) var appDelegate

    var body: some Scene {
        WindowGroup {
            ContentView()
                .spezi(appDelegate)
        }
    }
    
}

struct ContentView: View {
  

    var body: some View {
        Text("Hello, world!")
            .onAppear {
               
            }
    }
        
   
}
