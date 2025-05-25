# Cephable C++ Virtual Controller Samples

There are 2 different samples in this directory:

1. `simple`: Uses low-level HTTP and websockets to connect to the signalr hub, but requires that you've already made a user device and device token. Given the challenging support from the official `microsoft-signalr` package, this is the recommended approach. This sample uses cpprestsdk directly.
2. `e2e`: Handles authenticating a user with a local server to handle the redirect response, then uses the `microsoft-signalr` package to connect to the hub.
3. `winhttp`: a work in progress sample using winhttp libraries to directly make the single negotiate call over http GET then handling the info to attempt the websocket connection.