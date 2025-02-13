import SwiftUI

struct ContentView: View {

    var body: some View {
        VStack {
            Text("Test")
                .font(.largeTitle)
                .padding()

            Button("Refresh Message") {
            }
            .padding()
            .background(Color.blue)
            .foregroundColor(.white)
            .cornerRadius(10)
        }
    }
}

#Preview {
    ContentView()
}
