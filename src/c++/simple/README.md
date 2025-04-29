# Simple Cephable C++ Device Sample

This sample shows how to connect to the device hub and receive commands but assumes you have already used the Cephable API to authenticate a user and create a user device and token.

## Requirements

- vcpkg for installing `cpprestsdk` and `uwebsockets`
- Project setup in Cephable Portal: https://portal.cephable.com 
- Create user device and token in Cephable API: https://services.cephable.com/swagger

## Steps

This demo handles the signalr connections natively with HTTP and websockets and does the following:

1. Takes the Device Token and negotiates the connection via HTTP
2. Takes the HTTP negotiation response and makes an HTTP request to the hub endpoint
3. Makes the secure websocket connection to the hub
4. Sends the VerifySelf event to the Hub
5. Logs every low level message received