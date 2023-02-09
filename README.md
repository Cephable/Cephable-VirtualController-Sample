# Enabled Play Virtual Controller Sample
A sample console application that connects to the Enabled Play virtual controller hub for receiving commands and inputs from the Enabled Play app

## Get Started

To try this sample out, you'll first need to create an Enabled Play account by downloading the Enabled Play app from https://enabledplay.com/apps


You'll then need to request a licensed API client ID, key, and virtual controller device type ID from Enabled Play. Contact support@enabledplay.com for more information.

You can then use the Enabled Play API at https://services.enabledplay.com/swagger to generate a new UserDevice for your user and then generate a new UserDevice Token for that device. This token is what you need to paste in the console when running the sample.

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
