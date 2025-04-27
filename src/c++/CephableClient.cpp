#include <thread>
#include <cstdlib>
#include <sstream>
#include <future>
#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>

#include "signalrclient/hub_connection.h"
#include "signalrclient/hub_connection_builder.h"
#include "signalrclient/signalr_value.h"

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace web::http::experimental::listener;

std::string authenticateWithBrowser(const std::string& apiUrl, const std::string& authClientId) {
    std::string authCode;
    std::promise<std::string> authCodePromise;
    auto authCodeFuture = authCodePromise.get_future();

    // Start a local HTTP server to capture the auth code
    http_listener listener("http://localhost:8080/callback");
    listener.support(methods::GET, [&authCodePromise](http_request request) {
    auto query = uri::split_query(request.request_uri().query());
        if (query.find("code") != query.end()) {
            authCodePromise.set_value(query["code"]);
            request.reply(status_codes::OK, "Authentication successful. You can close this window.");
        } else {
            request.reply(status_codes::BadRequest, "Missing auth code.");
        }
    });

    listener.open().wait();

    // Open the browser for user authentication
    std::string authUrl = apiUrl + "/signin?client_id=" + authClientId + "&redirect_uri=http://localhost:8080/callback";
    std::string command = "open " + authUrl;
    std::system(command.c_str());

    // Wait for the auth code
    authCode = authCodeFuture.get();

    listener.close().wait();

    // Exchange the auth code for a bearer token
    http_client client(apiUrl);
    uri_builder builder("/signin/token");
    builder.append_query("grant_type", "code");
    builder.append_query("code", authCode);
    builder.append_query("redirect_uri", "http://localhost:8080/callback");
    builder.append_query("client_id", authClientId);

    http_request tokenRequest(methods::POST);
    tokenRequest.headers().add(U("Content-Type"), U("application/x-www-form-urlencoded"));
    tokenRequest.set_request_uri(builder.to_uri());

    auto response = client.request(tokenRequest).get();
    if (response.status_code() == status_codes::OK) {
        auto responseBody = response.extract_json().get();
        return responseBody[U("access_token")].as_string();
    } else {
        throw std::runtime_error("Failed to exchange auth code for token.");
    }
}

std::string createDeviceWithBearerToken(const std::string& apiUrl, const std::string& bearerToken, const std::string& deviceTypeId) {
    http_client client(apiUrl);
    uri_builder createDeviceBuilder("/api/Device/userDevices/new/" + deviceTypeId);

    http_request createDeviceRequest(methods::POST);
    createDeviceRequest.headers().add(U("Authorization"), U("Bearer " + bearerToken));
    createDeviceRequest.headers().add(U("Content-Type"), U("application/json"));

    json::value devicePayload;
    devicePayload[U("name")] = json::value::string(U("Sample C++ Device"));
    createDeviceRequest.set_body(devicePayload);
    createDeviceRequest.set_request_uri(createDeviceBuilder.to_uri());

    // Make the first call to create the device
    auto createDeviceResponse = client.request(createDeviceRequest).get();
    if (createDeviceResponse.status_code() == status_codes::OK) {
        auto createDeviceResponseBody = createDeviceResponse.extract_json().get();
        auto userDeviceId = createDeviceResponseBody[U("id")].as_string();

        // Make the second call to get the device token
        uri_builder getTokenBuilder("/api/Device/userDevices/" + userDeviceId + "/tokens");
        http_request getTokenRequest(methods::POST);
        getTokenRequest.headers().add(U("Authorization"), U("Bearer " + bearerToken));
        getTokenRequest.set_request_uri(getTokenBuilder.to_uri());

        auto getTokenResponse = client.request(getTokenRequest).get();
        if (getTokenResponse.status_code() == status_codes::OK) {
            auto getTokenResponseBody = getTokenResponse.extract_json().get();
            return getTokenResponseBody[U("deviceToken")].as_string();
        } else {
            throw std::runtime_error("Failed to retrieve device token.");
        }
    } else {
        throw std::runtime_error("Failed to create device.");
    }
}

void connectToSignalRWithDeviceToken(const std::string& signalRUrl, const std::string& deviceToken) {
    auto connection = signalr::hub_connection_builder::create(signalRUrl).build();

    connection.on("DeviceCommand", [](const signalr::value& message) {
        std::cout << "Received command: " << message.as_string() << std::endl;
    });


    std::promise<void> startTask;
    connection.start([&connection, &startTask](std::exception_ptr exception)
    {
        if (exception)
        {
            try
            {
                std::rethrow_exception(exception);
            }
            catch (const std::exception & ex)
            {
                std::cout << "exception when starting connection: " << ex.what() << std::endl;
            }
        }
        startTask.set_value();
    });
    

    startTask.get_future().get();

    // Keep the connection alive
    std::this_thread::sleep_for(std::chrono::minutes(10));
}

int main() {
    const std::string apiUrl = "https://services.cephable.com";
    const std::string deviceTypeId = "[paste your device type ID here]";
    const std::string authClientId = "[paste your auth client ID here]";
    try {
        std::string bearerToken = authenticateWithBrowser(apiUrl, authClientId);
        std::cout << "Bearer token: " << bearerToken << std::endl;

        std::string deviceToken = createDeviceWithBearerToken(apiUrl, bearerToken, deviceTypeId);
        std::cout << "Device token: " << deviceToken << std::endl;

        const std::string signalRUrl = apiUrl + "/device";
        connectToSignalRWithDeviceToken(signalRUrl, deviceToken);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}