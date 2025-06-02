# WORK IN PROGRESS NOTICE

This project is currently a work in progress, and may not work end to end on all devices yet.

Currently this project uses WinHTTP natively to make a negotiation GET request to the Cephable services API to retrieve the correct Device Hub server to connect to and an access token to authenticate against it which is then used to make a GET request with a websocket upgrade to connect to the hub.

Current issues:

- The RetrieveResponse call fails with an error code of 12152 implying the server sent back an unexpected/malformed response. Current hunch is that this is due to some unknown TLS handshake issues from the winhttp client
- The signalr server traces indicate that the server receives the request correctly, generates a connection, but then receives a HandShake Cancelled event which does not necessarly mean a TLS handshake, but rather a SignalR handshake. The server is expecting the client to start the websocket connection from the response sent and then send the first payload (which is wired up in the code), but the client cannot proceed with the bad response.

# WinHTTP c++ Virtual Controller Demo

This demo uses winhttp and visual studio to make a 2 step connection to the Cephable Device Hub. To get started, use the Cephable portal to create an Auth Client and device, then use the Cephable API at https://services.cephable.com/swagger to create a UserDevice and UserDeviceToken. You'll then use that token in this code to create your request and connect.