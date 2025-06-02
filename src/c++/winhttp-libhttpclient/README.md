# WinHTTP with libHttpClient c++ Virtual Controller Demo

This demo uses winhttp for HTTP requests but libHttpClient for the websocket handshake and connection and visual studio to make a 2 step connection to the Cephable Device Hub. To get started, use the Cephable portal to create an Auth Client and device, then use the Cephable API at https://services.cephable.com/swagger to create a UserDevice and UserDeviceToken. You'll then use that token in this code to create your request and connect.

You can of course, replace the winhttp process and run the entire thing through libHttpClient since it has an even more robust HTTP process, but this was built on top of the samples at `../winhttp` in this repository in order to provide an alternative websocket connection that works.

This sample is meant for developers building in C++ on Microsoft platforms such as:
- win32
- UWP
- Xbox

- https://github.com/microsoft/libHttpClient

Websocket implementation originally taken from https://github.com/microsoft/libHttpClient/tree/main/Samples/Win32WebSocket and adapted to the SignalR standard.