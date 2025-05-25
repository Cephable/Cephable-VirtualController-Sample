# Browser Raw Example

This demo connects to the Cephable Device Hub without using the SignalR Javascript SDK. Instead it manually makes the negotiate request and the low level websocket connection. This is a great sample for referencing against low-level samples like the c++ samples in this repository. To get started, use the Cephable portal to create an Auth Client and device, then use the Cephable API at https://services.cephable.com/swagger to create a UserDevice and UserDeviceToken. You'll then use that token in this code to create your request and connect.

Monitor the network tab to see the SignalR heartbeat and events come through.