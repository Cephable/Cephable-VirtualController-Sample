# Cephable Virtual Controller Samples
A sample console application that connects to the Cephable virtual controller hub for receiving commands and inputs from the Cephable app
There are 3 different examples in this repository: Browser/JavaScript, Android/Kotlin, and Console/C#/.NET


To try these sample out, you'll first need to create an Cephable account by downloading the Cephable app from https://cephable.com/download

You'll then need to request a licensed API client ID, key, and virtual controller device type ID from Cephable. Contact support@cephable.com for more information.

## Get Started - Browser / JavaScript

To use the browser example, run `npm run start` from the `browser` folder to start a node server at http://localhost:3000

From here you need your client ID to paste in the field and then click "Sign In" to be taken to the Cephable login screen. Once you login, you will be redirected back to your localhost app which will then use the Cephable API to create a new virtual controller, controller token, and connect to the controller hub.

Once it is connected, it is ready to receive commands. Open the console in the browser, and then open the virtual controller from the Cephable app, give it a profile, and send commands. You'll see the commands log to the console.

## Get Started - Android / Kotlin

To use the Android example, open the gradle project in Android Studio and ensure you have >= SDK version 31 installed for the Android SDK.
In the `MainActivity.kt` you can set your client ID acquired from the Cephable team.

Then you can run the app on a local device or emulator. Simply tap the "Sign In" button, sign in with your Cephable account and the app will generate an Cephable Virtual Controller in your app and wait for commands. You can then run both this app and the Cephable app at the same time on any Android device. Use Cephabley expression controls or hotkeys to send commands and see the latest received command appear in the sample app. You can also a secondary device with the Cephable app to send commands to your app.


## Get Started - iOS / Swift

To use the iOS example, open the project in the latest version XCode .
In the `ContentView.swift` you can set your client ID acquired from the Cephable team.

Then you can run the app on a local device or emulator. Simply tap the "Sign In" button, sign in with your Cephable account and the app will generate an Cephable Virtual Controller in your app and wait for commands. You can then run both this app and the Cephable app at the same time on any iOS device or across devices. Use Cephable expression controls or hotkeys to send commands and see the latest received command appear in the sample app. You can also a secondary device with the Cephable app to send commands to your app.

## Get Started - C# / .NET


You can then use the Cephable API at https://services.cephable.com/swagger to generate a new UserDevice or use the Browser example and get the values from the `localStorage` to use for your user and then generate a new UserDevice Token for that device. This token is what you need to paste in the console when running the sample. The browser example generates both a device and a token for you to use too.

When you add a valid token in the sample, it will connect to the virtual controller hub and allow you to send commands from the Cephable app via face expressions, gestures, virtual buttons, and more.

## Other Features

The virtual controller hub can do more than just receive commands, you can also subscribe to other events including:
- `DeviceProfileUpdate`
- `DeviceSettingsUpdate`
- `DeviceType`
- `StartListening`
- `StopListening`

You can also send events over the hub to get the latest configuration information and update statuses including:

- `connection.SendAsync("GetLatestDevice")`
- `connection.SendAsync("VerifySelf")`
- `connection.SendAsync("DeviceStartedListening")`
- `connection.SendAsync("DeviceStoppedListening")`

Checkout the tutorials at https://cephable.com/tutorials for how to use the Cephable app to build profiles and send commands
