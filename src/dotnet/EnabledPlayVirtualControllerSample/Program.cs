using Microsoft.AspNetCore.SignalR.Client;

Console.WriteLine("Enter your device token (you'll need to get this from the Cephable API on behalf of your user account at https://services.cephable.com/swagger): ");
string deviceToken = Console.ReadLine();

var connection = new HubConnectionBuilder()
    .WithUrl("https://services.cephable.com/device", options =>
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

// Connection started, indicate to hub that we are listening
await connection.InvokeAsync("VerifySelf");

Console.WriteLine("Connected to hub. Waiting for commands... ");
Console.WriteLine("Press enter to exit.");
Console.ReadLine();