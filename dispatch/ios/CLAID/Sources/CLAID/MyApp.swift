// import SwiftUI

// @main
// struct MyApp: App {
//     // Optionally store the result pointer if you need to reference it later:
//     let coreHandle: UnsafeMutableRawPointer?

//     init() {
//         // Call your C function as soon as the app starts.
//         coreHandle = startCore(
//             socketPath: "/tmp/my_socket",
//             configFile: "/etc/my_config",
//             hostID: "host123",
//             userID: "user456",
//             deviceID: "dev789"
//         )
//         // You could do more initialization here...
//     }

//     var body: some Scene {
//         WindowGroup {
//             ContentView()
//                 // If you prefer, you could use .onAppear in a top-level view:
//                 // .onAppear {
//                 //     startCore(...)
//                 // }
//         }
//     }
// }