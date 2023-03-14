//
//  ContentView.swift
//  VirtualControllerDemo
//
//  Created by Alex Dunn on 3/13/23.
//

import SwiftUI

struct ContentView: View {
    
    private let clientId = "your-client-id"
    private let authRedirectUri = "enabledplay-samples://"
    private let deviceTypeId = "3ae3d1ed-97b7-4572-a57a-00d4724270a0" // Change this to your device type id
    private let state = UUID().uuidString
    @State private var accessToken: String? = nil
    @State private var refreshToken: String? = nil
    @State private var userDeviceId: String? = nil
    @State private var deviceToken: String? = nil
    @State private var connectionStatus: String = ""
    @State private var command: String = "This is where your latest command will be"
    
    // TODO: handle signalr setup
    //private var hubConnection: HubConnection?
    
    var body: some View {
        VStack {
            Text(connectionStatus)
                .padding()
            
            Button(action: {
                if accessToken != nil {
                    signOut()
                    return
                }
                signIn()
            }, label: {
                if accessToken != nil {
                    Text("Sign Out")
                } else {
                    Text("Sign In")
                }
            })
            Text(command)
            
        }
        .onAppear(perform: {
            loadState()
            updateState()
            if deviceToken != nil {
                // TODO
                //connect()
            }
        })
    }
    
    func signOut() {
        accessToken = nil
        refreshToken = nil
        userDeviceId = nil
        deviceToken = nil
        UserDefaults.standard.set("", forKey: "accessToken")
        UserDefaults.standard.set("", forKey: "refreshToken")
        UserDefaults.standard.set("", forKey: "userDeviceId")
        UserDefaults.standard.set("", forKey: "deviceToken")
    }
    
    func signIn() {
        let authUri = URL(string: "https://services.enabledplay.com/signin")!
            .appending(queryItems:[
                URLQueryItem(name:"client_id", value: clientId),
                URLQueryItem(name:"state",value: state),
                URLQueryItem(name:"redirect_uri", value: authRedirectUri)
            ])
        
        UIApplication.shared.open(authUri)
    }
    
    private func setStatusText(text: String) {
        connectionStatus = text
    }
    
    private func loadState() {
        accessToken = UserDefaults.standard.string(forKey: "accessToken")
        refreshToken = UserDefaults.standard.string(forKey: "refreshToken")
        userDeviceId = UserDefaults.standard.string(forKey: "userDeviceId")
        deviceToken = UserDefaults.standard.string(forKey: "deviceToken")
    }
    
    private func updateState() {
        if accessToken != nil {
            connectionStatus = "Signed In"
        } else {
            connectionStatus = "Signed Out"
        }
//        
//        if hubConnection?.state == .connected {
//            connectionStatus = "Connected and Ready for Commands. Use the Enabled Play app to start sending commands with virtual buttons or expression controls"
//        } else {
//            connectionStatus = "Disconnected. Try restarting the app or signing out to try again"
//        }
    }
    
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
