using Microsoft.AspNetCore.SignalR.Client;
using Newtonsoft.Json;

Console.WriteLine("Enter your device token (you'll need to get this from the Enabled Play API on behalf of your user account at https://services.enabledplay.com/swagger): ");
string deviceToken = Console.ReadLine();

var connection = new HubConnectionBuilder()
    .WithUrl("https://services.enabledplay.com/device", options =>
    {
        options.AccessTokenProvider = () => Task.FromResult(deviceToken);
        options.Headers.Add("X-Device-Token", deviceToken);
    })
    .Build();

connection.On<string>("DeviceCommand", (command) =>
{
    Console.WriteLine("Received command: " + command);
});


await connection.StartAsync();

await connection.SendAsync("VerifySelf");

Console.WriteLine("Connected to hub. Waiting for commands... ");
Console.WriteLine("Press enter to exit.");
Console.ReadLine();