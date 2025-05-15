#include <cpprest/http_client.h>
#include <uwebsockets/App.h>
#include <nlohmann/json.hpp> // A convenient library for JSON handling
#include <iostream>
#include <string>

using namespace web::http;
using namespace web::http::client;
using json = nlohmann::json; // Alias for JSON usage

void negotiate_and_connect(const std::string& deviceToken) {
    try {
        // Step 1: Negotiate
        http_client client(U("https://services.cephable.com/device/"));
        uri_builder builder(U("negotiate"));
        builder.append_query(U("negotiateVersion"), U("1"));

        http_request request(methods::POST);
        request.set_request_uri(builder.to_uri());
        request.headers().add(U("X-DEVICE-TOKEN"), utility::conversions::to_string_t(deviceToken));
        request.headers().add(U("Authorization"), U("Bearer ") + utility::conversions::to_string_t(deviceToken));

        auto response = client.request(request).get();
        if (response.status_code() != status_codes::OK) {
            std::cerr << "Failed to negotiate: " << response.status_code() << std::endl;
            return;
        }

        auto jsonBody = response.extract_json().get();
        std::string baseUrl = jsonBody[U("url")].as_string();
        // Extract the host and query part from the URL
        size_t hostEnd = baseUrl.find('/', 8); // Skip https://
        size_t queryStart = baseUrl.find('?');
        
        std::string host = (hostEnd != std::string::npos) ? baseUrl.substr(0, hostEnd) : baseUrl;
        std::string query = (queryStart != std::string::npos) ? baseUrl.substr(queryStart) : "";
        
        // Construct the new URL with /client/negotiate
        std::string url = host + "/client/negotiate" + query;
        std::string accessToken = jsonBody[U("accessToken")].as_string();

        // Step 2: Authenticate to the hub
        http_request hubRequest(methods::POST);
        hubRequest.set_request_uri(utility::conversions::to_string_t(url));
        hubRequest.headers().add(U("X-DEVICE-TOKEN"), utility::conversions::to_string_t(deviceToken));
        hubRequest.headers().add(U("Authorization"), U("Bearer ") + utility::conversions::to_string_t(accessToken));

        auto hubResponse = client.request(hubRequest).get();
        if (hubResponse.status_code() != status_codes::OK) {
            std::cerr << "Failed to authenticate with hub: " << hubResponse.status_code() << std::endl;
            return;
        }

        auto hubJsonBody = hubResponse.extract_json().get();
        std::string connectionId = hubJsonBody[U("connectionId")].as_string();
        std::string connectionToken = hubJsonBody[U("connectionToken")].as_string();

        // Step 3: Connect via WebSocket using uWebSockets
        std::string wsUrl = baseUrl.replace(0, 4, "wss") + "&id=" + connectionToken + "&access_token=" + accessToken;

        uWS::App()
            .ws("/*", {
                .open = [](auto* ws) {
                    std::cout << "WebSocket connection established!" << std::endl;

                    // Step 4: Invoke the VerifySelf event
                    json verifySelfPayload = {
                        {"arguments", json::array()},
                        {"invocationId", "0"},
                        {"target", "VerifySelf"},
                        {"type", 1}
                    };
                    ws->send(verifySelfPayload.dump(), uWS::OpCode::TEXT);
                    std::cout << "Sent VerifySelf event." << std::endl;
                },
                .message = [](auto* ws, std::string_view message, uWS::OpCode opCode) {
                    std::cout << "Message received: " << message << std::endl;
                },
                .close = [](auto* ws, int code, std::string_view message) {
                    std::cout << "Connection closed: " << message << std::endl;
                }
            })
            .connect(wsUrl, {}, {}, [&](auto* ws, auto* connectError) {
                if (connectError) {
                    std::cerr << "WebSocket connection error: " << connectError->reason << std::endl;
                }
            })
            .run();
    } catch (const std::exception& ex) {
        std::cerr << "Exception occurred: " << ex.what() << std::endl;
    }
}

int main() {
    std::string deviceToken = "your_device_token_here";
    negotiate_and_connect(deviceToken);
    return 0;
}
