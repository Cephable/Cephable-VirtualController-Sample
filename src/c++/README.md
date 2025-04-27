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
Here's an example of the full json body received from the websocket for `DeviceCommand`:

```json
{
    "type": 1,
    "target": "DeviceCommand",
    "arguments": [
        "Move Forward",
        {
            "id": "2c13dc42-6e06-4e03-a780-798cb86cef1a",
            "name": "Move Forward",
            "commands": [
                "move forward",
                "go forward",
                "head_tilt_forward"
            ],
            "events": [
                {
                    "eventType": "KeyPress",
                    "keys": [
                        "w"
                    ],
                    "holdTimeMilliseconds": 0,
                    "isKeyLatch": false,
                    "typedPhrase": null,
                    "mouseMoveX": null,
                    "mouseMoveY": null,
                    "mouseMoveScroll": null,
                    "joystickLeftMoveX": null,
                    "joystickLeftMoveY": null,
                    "joystickRightMoveX": null,
                    "joystickRightMoveY": null,
                    "outputSpeech": null,
                    "audioFileUrl": null,
                    "deviceTypeCustomActionId": null,
                    "deviceTypeId": null,
                    "additionalInputContent": null
                }
            ]
        }
    ]
}
```

## Issues with microsoft-signalr package?

The C++ microsoft-signalr vcpkg is in a preview state, but you do not need the entire package to connect to the hub and receive commands. You can also implement the connection using low level HTTP and websocket requests by following the steps:

1. HTTP POST to the negotiate endpoint:
`https://services.cephable.com/device/negotiate?negotiateVersion=1` with the `X-DEVICE-TOKEN` HTTP Header and/or wit hthe `Authorization` header with the value of `Bearer {your device token}`

Here you will receive the json body payload with more information on how to connect to the websocket hub. Ex:
```json
{
    "negotiateVersion": 0,
    "url": "https://enabled-prod.service.signalr.net/client/?hub=devicehub&asrs.op=%2Fdevice&negotiateVersion=1&asrs_request_id=g1vFtD4AAAA%3D&asrs_lang=en-US&asrs_ui_lang=en-US",
    "accessToken": "[some token]",
    "availableTransports": []
}
```

2. Make a POST request to the `url` property from the response but this time, change the `Authorization` HTTP header to the new access token value from the first response. Be sure to also send the `X-Device-Token` header as well but wit the original device token value.
You'll receive a JSON response body with a connection ID and token as well as information about the transport types it can use such as websockets. Ex:
```json
{
    "negotiateVersion": 1,
    "connectionId": "uVuZhiD54MqKCmxiyi36CweHFWuwK02",
    "connectionToken": "NgYK8WrQ7HaSqzNZNBCotAeHFWuwK02",
    "availableTransports": [
        {
            "transport": "WebSockets",
            "transferFormats": [
                "Text",
                "Binary"
            ]
        },
        {
            "transport": "ServerSentEvents",
            "transferFormats": [
                "Text"
            ]
        },
        {
            "transport": "LongPolling",
            "transferFormats": [
                "Text",
                "Binary"
            ]
        }
    ]
}
```

3. Now make the websocket connection! This will use a uri based on the received url from the first request as well as appending the access token and connection token info. Ex:
```
wss://enabled-prod.service.signalr.net/client/?hub=devicehub&asrs.op=%2Fdevice&negotiateVersion=1&asrs_request_id=g1vFtD4AAAA%3D&asrs_lang=en-US&asrs_ui_lang=en-US&id=NgYK8WrQ7HaSqzNZNBCotAeHFWuwK02&access_token={your original access token}
```

4. Observe the events being received over the websocket connection and invoke methods. Ex:

- `VerifySelf` call to get latest device info, send:
```json
{"arguments":[],"invocationId":"0","target":"VerifySelf","type":1}
```

- `DeviceCommand` event received:
```json
{
    "type": 1,
    "target": "DeviceCommand",
    "arguments": [
        "Move Forward",
        {
            "id": "2c13dc42-6e06-4e03-a780-798cb86cef1a",
            "name": "Move Forward",
            "commands": [
                "move forward",
                "go forward",
                "head_tilt_forward"
            ],
            "events": [
                {
                    "eventType": "KeyPress",
                    "keys": [
                        "w"
                    ],
                    "holdTimeMilliseconds": 0,
                    "isKeyLatch": false,
                    "typedPhrase": null,
                    "mouseMoveX": null,
                    "mouseMoveY": null,
                    "mouseMoveScroll": null,
                    "joystickLeftMoveX": null,
                    "joystickLeftMoveY": null,
                    "joystickRightMoveX": null,
                    "joystickRightMoveY": null,
                    "outputSpeech": null,
                    "audioFileUrl": null,
                    "deviceTypeCustomActionId": null,
                    "deviceTypeId": null,
                    "additionalInputContent": null
                }
            ]
        }
    ]
}
```