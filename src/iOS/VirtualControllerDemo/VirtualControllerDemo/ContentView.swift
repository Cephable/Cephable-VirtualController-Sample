//
//  ContentView.swift
//  VirtualControllerDemo
//
//  Created by Alex Dunn on 3/13/23.
//

import SwiftUI
import SignalRClient

struct ContentView: View {
    
    private let clientId = "[your-client-id]"
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
    @State private var hubConnection: HubConnection?
    
    init(authCode: String?) {
        if(authCode != nil && accessToken == nil) {
            exchangeCodeForTokens(code: authCode!)
        }
    }
    
    
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
                connect()
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
        let authUri = URL(string: "https://services.cephable.com/signin")!
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
        if accessToken != nil && deviceToken != nil {
            connectionStatus = "Signed In"
        } else {
            connectionStatus = "Signed Out - sign in with your Cephable account to register a new virtual controller and get started"
        }
        //
        //        if hubConnection?.state == .connected {
        //            connectionStatus = "Connected and Ready for Commands. Use the Cephable app to start sending commands with virtual buttons or expression controls"
        //        } else {
        //            connectionStatus = "Disconnected. Try restarting the app or signing out to try again"
        //        }
    }
    
    private func exchangeCodeForTokens(code: String) {
        let url = URL(string: "https://services.cephable.com/signin/token")!
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.setValue("application/x-www-form-urlencoded", forHTTPHeaderField: "Content-Type")
        
        let parameters: [String: String] = [
            "client_id": clientId,
            "code": code,
            "grant_type": "code",
            "redirect_uri": authRedirectUri
        ]
        request.httpBody = parameters
            .map({ (key, value) in "\(key)=\(value)" })
            .joined(separator: "&")
            .data(using: .utf8)
        
        let task = URLSession.shared.dataTask(with: request) { data, response, error in
            if let error = error {
                // Handle the failure case
                self.setStatusText(text: error.localizedDescription)
                print(error)
                return
            }
            
            guard let data = data,
                  let httpResponse = response as? HTTPURLResponse,
                  httpResponse.statusCode == 200 else {
                // Handle the non-successful response case
                return
            }
            
            if let json = try? JSONSerialization.jsonObject(with: data, options: []) as? [String: Any],
                let accessToken = json["access_token"] as? String,
                let refreshToken = json["refresh_token"] as? String {
               
                saveValue(key: "accessToken", value: accessToken)
                saveValue(key: "refreshToken", value: refreshToken)
                createUserDevice(deviceTypeId: deviceTypeId, accessToken: accessToken)
                
                
            }
        }
        task.resume()
    }
    
    
    private func createUserDevice(deviceTypeId: String, accessToken: String) -> Void{
        let url = URL(string: "https://services.cephable.com/api/Device/userDevices/new/\(deviceTypeId)?name=iOS-Sample")!
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.setValue("Bearer \(accessToken)", forHTTPHeaderField: "Authorization")
        
        let task = URLSession.shared.dataTask(with: request) { data, response, error in
            if let error = error {
                print(error)
                return
            }
            
            guard let data = data,
                  let httpResponse = response as? HTTPURLResponse,
                  httpResponse.statusCode == 200 else {
                return
            }
            
            if let json = try? JSONSerialization.jsonObject(with: data, options: []) as? [String: Any] {
                let userDeviceId = json["id"] as? String
                self.userDeviceId = userDeviceId
                self.saveValue(key: "userDeviceId", value: userDeviceId!)
                
                
                createUserDeviceToken(userDeviceId: userDeviceId!, accessToken: accessToken)
            }
        }
        task.resume()
    }
    
    
    private func createUserDeviceToken(userDeviceId: String, accessToken: String) -> Void{
        
        let url = URL(string: "https://services.cephable.com/api/Device/userDevices/\(userDeviceId)/tokens")!
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.setValue("Bearer \(accessToken)", forHTTPHeaderField: "Authorization")
        
        let task = URLSession.shared.dataTask(with: request) { data, response, error in
            if let error = error {
                print(error)
                return
            }
            
            guard let data = data,
                  let httpResponse = response as? HTTPURLResponse,
                  httpResponse.statusCode == 200 else {
                return
            }
            
            if let json = try? JSONSerialization.jsonObject(with: data, options: []) as? [String: Any] {
                let deviceToken = json["token"] as? String
                self.deviceToken = deviceToken
                self.saveValue(key: "deviceToken", value: deviceToken!)
            }
        }
        task.resume()
    }
    
    func connect() -> Void {
        connectionStatus = "Connecting to device hub..."
        // TODO: handle when connected to not try again
        if(deviceToken == nil || deviceToken == ""){
            if(userDeviceId != nil && userDeviceId!.count > 0) {
                createUserDeviceToken(userDeviceId: userDeviceId!, accessToken: self.accessToken!)
            }
            connectionStatus = "Unable to connect. Try signing out and back in again."
            return
        }
        hubConnection = HubConnectionBuilder(url: URL(string: "https://services.cephable.com/device")!)
            .withAutoReconnect().withHttpConnectionOptions(configureHttpOptions: { (options: HttpConnectionOptions) in
                options.headers = ["X-DEVICE-TOKEN": deviceToken!]
                options.accessTokenProvider = { return deviceToken! }
            })
            .withHubConnectionDelegate(delegate: HubDelegate(view: self))
            .build()
        
        hubConnection?.on(method: "DeviceCommand", callback: {(message: String) in
            print(message)
            command = message
        })
        
        hubConnection?.start()
        
    }
    
    func connectionComplete() {
        connectionStatus = "Connected"
        hubConnection?.invoke(method: "VerifySelf", invocationDidComplete: {err in
            print(err as Any)
        })
    }
    func connectionClosed() {
        connectionStatus = "Connection closed."
    }
    
    private func saveValue(key: String, value: String) -> Void{
        UserDefaults.standard.set(value, forKey: key)
    }
}

class HubDelegate: HubConnectionDelegate {
    var view: ContentView
    init(view: ContentView) {
        self.view = view
    }
    
    func connectionDidOpen(hubConnection: HubConnection) {
        view.connectionComplete()
    }
    
    func connectionDidFailToOpen(error: Error) {
        view.connectionClosed()
    }
    
    func connectionDidClose(error: Error?) {
        view.connectionClosed()
        
    }
    
    func connectionWillReconnect(error: Error) {
        
    }
    
    func connectionDidReconnect() {
        
            view.connectionComplete()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView(authCode: nil)
    }
}
