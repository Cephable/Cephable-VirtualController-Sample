# Cephable C++ Virtual Controller Sample

This sample demonstrates the steps to authenticate a Cephable user, use the Cephable REST API to register a virtual user device and a token for that device. It then uses SignalR to connect to the Cephable Device Hub which allows for a user to use the Cephable app to send commands to the device to then execute.

## Requirements

- A valid Cephable developer account: https://portal.cephable.com 
- A Cephable project with an Auth Client and custom Device Type
- `microsoft-signalr` client installed with `cpprestsdk[websocket]` feature installed as well

## Steps

This sample is meant to demonstrate the steps needed to implement a virtual controller, but is simplified to show the key steps all together:

1. Authenticate a Cephable User: Cephable uses OAuth code grant flows to authenticate a user. In this sample, we create a basic localhost HTTP server to wait for responses, but you can authenticate a user by whatever means you want - the main thing we need is the user's **Access Token** to be able to make authenticated requests to the Cephable API

2. Use the Cephable API (https://services.cephable.com/swagger) to register a user device and a token for this device. A Cephable `UserDevice` represents a connection that Cephable can send commands and actions to. 

3. Use the new `UserDevice`'s `token` to authenticate the virtual device to the Signalr Hub (Cephable Device Hub). SignalR uses `websockets` to maintain connections and receive commands. The primary command we show here is the `DeviceCommand` which receives 2 arguments - the command that was sent and the macro it is currently mapped to in the `UserDevice` `CurrentProfile` configuration.