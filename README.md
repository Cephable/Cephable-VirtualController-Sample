# Enabled Play Virtual Controller Sample
A sample console application that connects to the Enabled Play virtual controller hub for receiving commands and inputs from the Enabled Play app
There are 2 different examples in this repository: Browser/JavaScript and Console/C#/.NET


To try these sample out, you'll first need to create an Enabled Play account by downloading the Enabled Play app from https://enabledplay.com/apps

You'll then need to request a licensed API client ID, key, and virtual controller device type ID from Enabled Play. Contact support@enabledplay.com for more information.

## Get Started - Browser

To use the browser example, run `npm run start` from the `browser` folder to start a node server at http://localhost:3000

From here you need your client ID to paste in the field and then click "Sign In" to be taken to the Enabled Play login screen. Once you login, you will be redirected back to your localhost app which will then use the Enabled Play API to create a new virtual controller, controller token, and connect to the controller hub.

Once it is connected, it is ready to receive commands. Open the console in the browser, and then open the virtual controller from the Enabled Play app, give it a profile, and send commands. You'll see the commands log to the console.

## Get Started - C# / .NET


You can then use the Enabled Play API at https://services.enabledplay.com/swagger to generate a new UserDevice or use the Browser example and get the values from the `localStorage` to use for your user and then generate a new UserDevice Token for that device. This token is what you need to paste in the console when running the sample. The browser example generates both a device and a token for you to use too.

When you add a valid token in the sample, it will connect to the virtual controller hub and allow you to send commands from the Enabled Play app via face expressions, gestures, virtual buttons, and more.

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

Checkout the tutorials at https://enabledplay.com/tutorials for how to use the Enabled Play app to build profiles and send commands
