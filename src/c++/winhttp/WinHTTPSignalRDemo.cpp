
#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <random>
#include "websocket.h"

#pragma comment(lib, "winhttp.lib")
using namespace WinHttpWebSocketClient;

std::wstring affinityCookie = L"";
std::wstring asrsInstanceId = L"";

std::wstring ConvertToWide(const std::string &str)
{
    return std::wstring(str.begin(), str.end());
}

std::string ConvertToNarrow(const std::wstring &wstr)
{
    return std::string(wstr.begin(), wstr.end());
}
std::string generateRandomBase64String(size_t length) {
    const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::random_device rd;  // Seed for random number generator
    std::mt19937 gen(rd()); // Mersenne Twister RNG
    std::uniform_int_distribution<> dis(0, base64_chars.size() - 1);

    std::string random_base64;
    random_base64.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        random_base64 += base64_chars[dis(gen)];
    }

    return random_base64;
}
std::string UrlEncode(const std::string &value)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i)
    {
        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char)c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}
std::string HttpGet(const std::wstring &host, const std::wstring &path, const std::wstring &deviceToken, INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT)
{
    HINTERNET hSession = WinHttpOpen(L"SignalRClient/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

    std::wstring headers = L"X-DEVICE-TOKEN: " + deviceToken + L"\r\n";
    headers += L"Origin: console\r\n";
    WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);

    // Log request headers
    std::wcout << L"Request URL: https://" << host << path << std::endl;
    std::wcout << L"Request Headers:\n"
               << headers << std::endl;

    WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    WinHttpReceiveResponse(hRequest, NULL);


    // Log response headers
    wchar_t buffer[4096];
    DWORD bufferSize = sizeof(buffer);
    if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,
                            buffer, &bufferSize, WINHTTP_NO_HEADER_INDEX))
    {
        std::wcout << L"Response Headers:\n"
                   << buffer << std::endl;
    }
    else
    {
        std::wcerr << L"Failed to retrieve response headers. Error: " << GetLastError() << std::endl;
    }

    DWORD dwSize = 0;
    std::string response;
    do
    {
        DWORD dwDownloaded = 0;
        WinHttpQueryDataAvailable(hRequest, &dwSize);
        if (dwSize == 0)
            break;

        std::vector<char> buffer(dwSize + 1);
        WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
        buffer[dwDownloaded] = 0; // Null-terminate for safe printing
        response.append(buffer.data(), dwDownloaded);
    } while (dwSize > 0);

    // Log response body
    std::cout << "Response Body:\n"
              << response << std::endl;

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return response;
}

std::wstring ExtractWebSocketUrl(const std::string &jsonResponse)
{
    size_t urlPos = jsonResponse.find("\"url\":\"");
    if (urlPos == std::string::npos)
        return L"";

    size_t startPos = urlPos + 7;
    size_t endPos = jsonResponse.find("\"", startPos);
    std::string url = jsonResponse.substr(startPos, endPos - startPos);

    return ConvertToWide(url);
}

std::wstring ExtractAccessToken(const std::string &jsonResponse)
{
    size_t tokenPos = jsonResponse.find("\"accessToken\":\"");
    if (tokenPos == std::string::npos)
        return L"";

    size_t startPos = tokenPos + 15;
    size_t endPos = jsonResponse.find("\"", startPos);
    std::string token = jsonResponse.substr(startPos, endPos - startPos);

    return ConvertToWide(token);
}


void ConnectWebSocket(const std::wstring &fullUrl)
{
    URL_COMPONENTS urlComp = {sizeof(urlComp)};
    wchar_t hostName[256], urlPath[1024];
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = _countof(hostName);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = _countof(urlPath);
    WinHttpCrackUrl(fullUrl.c_str(), 0, 0, &urlComp);

    HINTERNET hSession = WinHttpOpen(L"SignalRClient/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    HINTERNET hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath, NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

    // Generate a random base64 string for Sec-WebSocket-Key
    std::string randomBase64 = generateRandomBase64String(22) + "==";
    std::wstring secWebSocketKey = ConvertToWide(randomBase64);
    
    std::wstring headers = L"Connection: Upgrade\r\n";
    headers += L"Upgrade: websocket\r\n";
    headers += L"Sec-WebSocket-Version: 13\r\n";
    headers += L"Sec-WebSocket-Key: " + secWebSocketKey + L"\r\n";
    //headers += L"Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n";
    headers += L"Pragma: no-cache\r\n";
    headers += L"Cache-Control: no-cache\r\n";
    headers += L"Origin: console\r\n";
    headers += L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/136.0.0.0 Safari/537.36 Edg/136.0.0.0\r\n";

    WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);

    std::wcout << L"Request Headers:\n"
               << headers << L"\n";

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
    {
        std::wcerr << L"SendRequest failed.\n";
        return;
    }
    if (!WinHttpReceiveResponse(hRequest, NULL))
    {
        DWORD error = GetLastError();
        std::wcerr << L"ReceiveResponse failed. Error code: " << error << L"\n";

        wchar_t errorMsg[512];
        FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorMsg, sizeof(errorMsg) / sizeof(wchar_t),
            NULL);
        std::wcerr << L"Error description: " << errorMsg << L"\n";
        return;
    }

    DWORD option = WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET;
    WinHttpSetOption(hRequest, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, &option, sizeof(option));

    HINTERNET hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, 0);
    if (!hWebSocket)
    {
        std::wcerr << L"WebSocket upgrade failed.\n";

        DWORD statusCode = 0;
        DWORD size = sizeof(statusCode);
        if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size, WINHTTP_NO_HEADER_INDEX))
        {
            std::wcerr << L"HTTP Status Code: " << statusCode << L"\n";
        }
        else
        {
            std::wcerr << L"Failed to retrieve HTTP status code.\n";
        }

        wchar_t buffer[1024];
        DWORD bufferSize = sizeof(buffer);
        if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, buffer, &bufferSize, WINHTTP_NO_HEADER_INDEX))
        {
            std::wcout << L"Response Headers:\n"
                       << buffer << L"\n";
        }
        else
        {
            std::wcerr << L"Failed to retrieve response headers.\n";
        }

        // Read the response body
        std::string responseBody;
        DWORD dwSize = 0;
        do
        {
            DWORD dwDownloaded = 0;
            WinHttpQueryDataAvailable(hRequest, &dwSize);
            if (dwSize == 0)
                break;

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

    const char *ping = "ping";
    WinHttpWebSocketSend(hWebSocket, WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, (PVOID)ping, strlen(ping));

    WinHttpWebSocketClose(hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
    WinHttpCloseHandle(hWebSocket);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
}

void ConnectWebSocket2(const std::wstring& fullUrl)
{
    // Windows Error Code
    DWORD errorCode;

    // Our certificate variable for secure connections
    PCCERT_CONTEXT pClientCertificate = NULL;

    // Buffer length
    const size_t bufferLength = 0x1000;

    // Error buffer
    WCHAR* buffer = NULL;

    // WebSocket message buffer
    CHAR* pMessageBuffer = NULL;


    // Use WinHttpOpen to obtain a session handle
    // The session should be opened in synchronous mode
    HINTERNET hSession = WinHttpOpen(L"MyApp", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    // Allocate error buffer
    buffer = (WCHAR*)malloc(sizeof(WCHAR) * bufferLength);
    if (buffer == NULL)
    {
        wprintf(L"%ls", L"Not enough memory\n");
        goto exit;
    }

    if (hSession == NULL)
    {
        errorCode = GetLastError();
        PrintLastError(errorCode, buffer, bufferLength, L"WinHttpOpen()");
        wprintf(L"%ls\n", buffer);
        goto exit;
    }

    // Define our WebSocket
    WebSocketClient webSocket;

    // Open a certificate to secure our connections
    //pClientCertificate = OpenCertificateByName(L"sullewarehouse.com", L"WebHosting", false);
    //if (pClientCertificate == NULL)
    //{
        //wprintf(L"%ls", L"Could not find the desired client certificate!\n");
    //}

    // Initialize our WebSocket
    // NOTE: If you do not provide the server with a certificate, data sent from the server will not be encrypted
    if (webSocket.Initialize(hSession, pClientCertificate) != NO_ERROR)
    {
        PrintLastError(webSocket.ErrorCode, buffer, bufferLength, L"WebSocket.Initialize()");
        wprintf(L"%ls\n", buffer);
        goto exit;
    }

    // Attempt to connect to the WebSocket server
    // NOTE: WinHTTP does not support "ws" or "wss" schemes in the URL, we can use "http" or "https" without issues
    if (webSocket.Connect(fullUrl, WEBSOCKET_SECURE_CONNECTION) != NO_ERROR)
    {
        wprintf(L"%ls\n", webSocket.ErrorDescription);
        if (webSocket.ErrorCode == 0x2F9A) {
            wprintf(L"%ls\n", L"Check if you have admin privileges");
        }
        goto exit;
    }

    wprintf(L"%ls", L"Connection was a success!\n");

    // Allocate message buffer
    pMessageBuffer = (CHAR*)malloc(0x1000);
    if (pMessageBuffer == NULL) {
        wprintf(L"%ls", L"Failed to allocate message buffer!\n");
        webSocket.Close(WINHTTP_WEB_SOCKET_CLOSE_STATUS::WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS);
        goto exit;
    }


exit:

    // Free message resources
    if (pMessageBuffer) {
        free(pMessageBuffer);
    }

    // Free WebSocket resources
    webSocket.Free();

    // Free certificate resources
    if (pClientCertificate) {
        CertFreeCertificateContext(pClientCertificate);
    }

    // Notify user that we are done
    wprintf(L"%ls", L"Program Exited!\n");

    // Exit
    return;
}
int main()
{
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
    if (queryPos != std::wstring::npos)
    {
        // Insert /client/negotiate before the query string
        negotiateUrl.insert(queryPos, L"negotiate");
    }
    else
    {
        // If no query string, just append /client/negotiate
        negotiateUrl += L"negotiate";
    }

    std::cout << "Azure SignalR URL: " << ConvertToNarrow(negotiateUrl) << std::endl;

    // Step 5: Encode access token
    std::wstring encodedAccessToken = ConvertToWide(UrlEncode(ConvertToNarrow(accessToken)));

    // Step 5: Construct final WebSocket URL
    std::wstring finalWebSocketUrl = azureSignalRUrl + L"&access_token=" + accessToken;
    std::cout << "Final WebSocket URL: " << ConvertToNarrow(finalWebSocketUrl) << std::endl;
    // Step 6: Connect via WebSocket
    ConnectWebSocket2(finalWebSocketUrl);

    return 0;
}
