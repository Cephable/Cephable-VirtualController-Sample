
#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "winhttp.lib")

std::wstring ConvertToWide(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

std::string ConvertToNarrow(const std::wstring& wstr) {
    return std::string(wstr.begin(), wstr.end());
}

std::string HttpGet(const std::wstring& host, const std::wstring& path, const std::wstring& deviceToken, INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT) {
    HINTERNET hSession = WinHttpOpen(L"SignalRClient/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

    std::wstring headers = L"X-DEVICE-TOKEN: " + deviceToken + L"\r\n";
    WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);
    
    // Log request headers
    std::wcout << L"Request URL: https://" << host << path << std::endl;
    std::wcout << L"Request Headers:\n" << headers << std::endl;

    WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    WinHttpReceiveResponse(hRequest, NULL);

    // Log response headers
    wchar_t buffer[4096];
    DWORD bufferSize = sizeof(buffer);
    if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, 
                          buffer, &bufferSize, WINHTTP_NO_HEADER_INDEX)) {
        std::wcout << L"Response Headers:\n" << buffer << std::endl;
    }
    else {
        std::wcerr << L"Failed to retrieve response headers. Error: " << GetLastError() << std::endl;
    }

    DWORD dwSize = 0;
    std::string response;
    do {
        DWORD dwDownloaded = 0;
        WinHttpQueryDataAvailable(hRequest, &dwSize);
        if (dwSize == 0) break;

        std::vector<char> buffer(dwSize + 1);
        WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
        buffer[dwDownloaded] = 0; // Null-terminate for safe printing
        response.append(buffer.data(), dwDownloaded);
    } while (dwSize > 0);
    
    // Log response body
    std::cout << "Response Body:\n" << response << std::endl;

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return response;
}

std::string HttpPost(const std::wstring& url, const std::wstring& deviceToken, const std::wstring& accessToken) {
    URL_COMPONENTS urlComp = { sizeof(urlComp) };
    wchar_t hostName[256], urlPath[1024];
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = _countof(hostName);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = _countof(urlPath);
    WinHttpCrackUrl(url.c_str(), 0, 0, &urlComp);

    HINTERNET hSession = WinHttpOpen(L"SignalRClient/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    HINTERNET hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", (urlComp.lpszUrlPath + std::wstring(L"/client/negotiate")).c_str(), NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

    std::wstring headers = L"Authorization: Bearer " + accessToken + L"\r\n";
    headers += L"X-DEVICE-TOKEN: " + deviceToken + L"\r\n";
    headers += L"Content-Type: application/json\r\n";

    WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);

    const wchar_t* postData = L"{}";
    WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)postData, wcslen(postData) * sizeof(wchar_t), wcslen(postData) * sizeof(wchar_t), 0);
    WinHttpReceiveResponse(hRequest, NULL);

    DWORD dwSize = 0;
    std::string response;
    do {
        DWORD dwDownloaded = 0;
        WinHttpQueryDataAvailable(hRequest, &dwSize);
        if (dwSize == 0) break;

        std::vector<char> buffer(dwSize + 1);
        WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
        response.append(buffer.data(), dwDownloaded);
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return response;
}

std::wstring ExtractWebSocketUrl(const std::string& jsonResponse) {
    size_t urlPos = jsonResponse.find("\"url\":\"");
    if (urlPos == std::string::npos) return L"";

    size_t startPos = urlPos + 7;
    size_t endPos = jsonResponse.find("\"", startPos);
    std::string url = jsonResponse.substr(startPos, endPos - startPos);

    return ConvertToWide(url);
}

std::wstring ExtractAccessToken(const std::string& jsonResponse) {
    size_t tokenPos = jsonResponse.find("\"accessToken\":\"");
    if (tokenPos == std::string::npos) return L"";

    size_t startPos = tokenPos + 15;
    size_t endPos = jsonResponse.find("\"", startPos);
    std::string token = jsonResponse.substr(startPos, endPos - startPos);

    return ConvertToWide(token);
}

std::wstring ExtractConnectionToken(const std::string& jsonResponse) {
    // Log the JSON response for debugging
    std::cout << "Parsing connection token from JSON response: " << jsonResponse << std::endl;
    size_t tokenPos = jsonResponse.find("\"connectionToken\":\"");
    if (tokenPos == std::string::npos) return L"";

    size_t startPos = tokenPos + 20;
    size_t endPos = jsonResponse.find("\"", startPos);
    std::string token = jsonResponse.substr(startPos, endPos - startPos);

    return ConvertToWide(token);
}

std::wstring ExtractConnectionId(const std::string& jsonResponse) {
    std::cout << "Parsing connection id from JSON response: " << jsonResponse << std::endl;
    size_t idPos = jsonResponse.find("\"connectionId\":\"");
    if (idPos == std::string::npos) return L"";

    size_t startPos = idPos + 16;
    size_t endPos = jsonResponse.find("\"", startPos);
    std::string id = jsonResponse.substr(startPos, endPos - startPos);

    return ConvertToWide(id);
}

void ConnectWebSocket(const std::wstring& fullUrl, const std::wstring& deviceToken) {
    URL_COMPONENTS urlComp = { sizeof(urlComp) };
    wchar_t hostName[256], urlPath[1024];
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = _countof(hostName);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = _countof(urlPath);
    WinHttpCrackUrl(fullUrl.c_str(), 0, 0, &urlComp);

    HINTERNET hSession = WinHttpOpen(L"SignalRClient/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    HINTERNET hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath, NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

    std::wstring headers = L"Connection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n";
    headers += L"X-DEVICE-TOKEN: " + deviceToken + L"\r\n";

    WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);

    std::wcout << L"Request Headers:\n" << headers << L"\n";

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        std::wcerr << L"SendRequest failed.\n";
        return;
    }
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        DWORD error = GetLastError();
        std::wcerr << L"ReceiveResponse failed. Error code: " << error << L"\n";
        
        wchar_t errorMsg[512];
        FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorMsg, sizeof(errorMsg)/sizeof(wchar_t),
            NULL);
        std::wcerr << L"Error description: " << errorMsg << L"\n";
        return;
    }
    HINTERNET hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, 0);
    if (!hWebSocket) {
        std::wcerr << L"WebSocket upgrade failed.\n";

        DWORD statusCode = 0;
        DWORD size = sizeof(statusCode);
        if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size, WINHTTP_NO_HEADER_INDEX)) {
            std::wcerr << L"HTTP Status Code: " << statusCode << L"\n";
        }
        else {
            std::wcerr << L"Failed to retrieve HTTP status code.\n";
        }

        wchar_t buffer[1024];
        DWORD bufferSize = sizeof(buffer);
        if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, buffer, &bufferSize, WINHTTP_NO_HEADER_INDEX)) {
            std::wcout << L"Response Headers:\n" << buffer << L"\n";
        }
        else {
            std::wcerr << L"Failed to retrieve response headers.\n";
        }

        // Read the response body
        std::string responseBody;
        DWORD dwSize = 0;
        do {
            DWORD dwDownloaded = 0;
            WinHttpQueryDataAvailable(hRequest, &dwSize);
            if (dwSize == 0) break;

            std::vector<char> bodyBuffer(dwSize + 1);
            WinHttpReadData(hRequest, bodyBuffer.data(), dwSize, &dwDownloaded);
            bodyBuffer[dwDownloaded] = 0; // Null-terminate
            responseBody.append(bodyBuffer.data(), dwDownloaded);
        } while (dwSize > 0);

        std::cout << "Response Body:" << std::endl;
        std::cout << responseBody << std::endl;

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return;
    }

    std::wcout << L"WebSocket connection established.\n";

    const char* ping = "ping";
    WinHttpWebSocketSend(hWebSocket, WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, (PVOID)ping, strlen(ping));

    WinHttpWebSocketClose(hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
    WinHttpCloseHandle(hWebSocket);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
}

int main() {
    std::wstring deviceToken = L"OTQxZjcwNTMtYzIzMi00MGE3LTkxYTUtNDVkOTc5ODUxN2QwfHx8N1IxOThdKTBkYmUhNGlEWEhHXkxZMnVjWkMoZDlt";

    // Step 1: Negotiate with API server
    std::string negotiateResponse = HttpGet(L"services.cephable.com", L"/device/negotiate?negotiateVersion=1", deviceToken);
    std::cout << "Negotiate Response: " << negotiateResponse << std::endl;

    // Step 2: Parse negotiateResponse to extract Azure SignalR URL and access token
    std::wstring azureSignalRUrl = ExtractWebSocketUrl(negotiateResponse);
    std::wstring accessToken = ExtractAccessToken(negotiateResponse);

    // Step 3: Second negotiation with Azure SignalR service
    std::wstring negotiateUrl = azureSignalRUrl;
    size_t queryPos = negotiateUrl.find(L'?');
    if (queryPos != std::wstring::npos) {
        // Insert /client/negotiate before the query string
        negotiateUrl.insert(queryPos, L"negotiate");
    } else {
        // If no query string, just append /client/negotiate
        negotiateUrl += L"negotiate";
    }
    std::cout << "Azure SignalR URL: " << ConvertToNarrow(negotiateUrl) << std::endl;
    std::string secondNegotiateResponse = HttpPost(negotiateUrl, deviceToken, accessToken);
    std::cout << "Second Negotiate Response: " << secondNegotiateResponse << std::endl;

    // Step 4: Parse secondNegotiateResponse to extract connection token
    std::wstring connectionToken = ExtractConnectionToken(secondNegotiateResponse);
    std::wstring connectionId = ExtractConnectionId(secondNegotiateResponse);
    std::cout << "Connection ID: " << ConvertToNarrow(connectionId) << std::endl;
    std::cout << "Connection Token: " << ConvertToNarrow(connectionToken) << std::endl;
    // Step 5: Construct final WebSocket URL
    std::wstring finalWebSocketUrl = azureSignalRUrl + L"&id=" + connectionToken + L"&access_token=" + accessToken;
    std::cout << "Final WebSocket URL: " << ConvertToNarrow(finalWebSocketUrl) << std::endl;
    // Step 6: Connect via WebSocket
    ConnectWebSocket(finalWebSocketUrl, deviceToken);

    return 0;
}
