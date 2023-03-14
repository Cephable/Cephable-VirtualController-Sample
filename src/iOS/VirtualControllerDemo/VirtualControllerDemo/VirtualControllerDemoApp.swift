//
//  VirtualControllerDemoApp.swift
//  VirtualControllerDemo
//
//  Created by Alex Dunn on 3/13/23.
//

import SwiftUI

@main
struct VirtualControllerDemoApp: App {
    @State private var authCode: String? = nil
    var body: some Scene {
        WindowGroup {
            ContentView(authCode: authCode).onOpenURL{ url in
                print(url)
                authCode = getQueryStringParameter(url: url.absoluteString, param: "code")
            }
        }
    }
    
    func getQueryStringParameter(url: String, param: String) -> String? {
      guard let url = URLComponents(string: url) else { return nil }
      return url.queryItems?.first(where: { $0.name == param })?.value
    }
}
