
#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <random>
#include <cstdlib>
#include <ctime>
#include "websocket.h"

#pragma comment(lib, "winhttp.lib")
using namespace WinHttpWebSocketClient;

std::wstring affinityCookie = L"";
std::wstring asrsInstanceId = L"";

std::wstring ConvertToWide(const std::string& str)
{
	return std::wstring(str.begin(), str.end());
}

std::string ConvertToNarrow(const std::wstring& wstr)
{
	return std::string(wstr.begin(), wstr.end());
}

// Simple Base64 encoding table
const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Base64 encode function
std::string base64Encode(const unsigned char* data, size_t len) {
	std::string encoded;
	int val = 0, valb = -6;
	for (size_t i = 0; i < len; ++i) {
		val = (val << 8) + data[i];
		valb += 8;
		while (valb >= 0) {
			encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6)
		encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
	while (encoded.size() % 4)
		encoded.push_back('=');
	return encoded;
}

// Generate a 16-byte random WebSocket key, base64 encoded
std::string generateRandomSecWebSocketKey() {
	unsigned char randomBytes[16];
	std::srand(static_cast<unsigned int>(std::time(nullptr))); // Seed the RNG

	for (int i = 0; i < 16; ++i)
		randomBytes[i] = static_cast<unsigned char>(std::rand() % 256);

	return base64Encode(randomBytes, 16);
}

std::string UrlEncode(const std::string& value)
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
std::string HttpGet(const std::wstring& host, const std::wstring& path, const std::wstring& deviceToken, INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT)
{
	HINTERNET hSession = WinHttpOpen(L"SignalRClient/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
	HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

	std::wstring headers = L"X-DEVICE-TOKEN: " + deviceToken + L"\r\n";
	headers += L"Origin: https://enabled-prod.service.signalr.net\r\n";
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

std::wstring ExtractWebSocketUrl(const std::string& jsonResponse)
{
	size_t urlPos = jsonResponse.find("\"url\":\"");
	if (urlPos == std::string::npos)
		return L"";

	size_t startPos = urlPos + 7;
	size_t endPos = jsonResponse.find("\"", startPos);
	std::string url = jsonResponse.substr(startPos, endPos - startPos);

	return ConvertToWide(url);
}

std::wstring ExtractAccessToken(const std::string& jsonResponse)
{
	size_t tokenPos = jsonResponse.find("\"accessToken\":\"");
	if (tokenPos == std::string::npos)
		return L"";

	size_t startPos = tokenPos + 15;
	size_t endPos = jsonResponse.find("\"", startPos);
	std::string token = jsonResponse.substr(startPos, endPos - startPos);

	return ConvertToWide(token);
}
void DumpWinHttpErrorResponse(HINTERNET hRequest)
{
	// Try to get HTTP status code
	DWORD statusCode = 0;
	DWORD size = sizeof(statusCode);

	if (WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX,
		&statusCode,
		&size,
		WINHTTP_NO_HEADER_INDEX)) {
		std::wcout << L"Status Code: " << statusCode << std::endl;
	}
	else {
		DWORD err = GetLastError();
		std::wcout << L"[!] Failed to get status code. Error: " << err << std::endl;
	}

	// Try to get raw headers
	DWORD headerSize = 0;
	WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_RAW_HEADERS_CRLF,
		WINHTTP_HEADER_NAME_BY_INDEX,
		NULL,
		&headerSize,
		WINHTTP_NO_HEADER_INDEX);

	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		std::wstring headers(headerSize / sizeof(wchar_t), 0);
		if (WinHttpQueryHeaders(hRequest,
			WINHTTP_QUERY_RAW_HEADERS_CRLF,
			WINHTTP_HEADER_NAME_BY_INDEX,
			&headers[0],
			&headerSize,
			WINHTTP_NO_HEADER_INDEX)) {
			std::wcout << L"--- Raw Headers ---\n" << headers.c_str() << std::endl;
		}
		else {
			std::wcout << L"[!] Failed to read raw headers. Error: " << GetLastError() << std::endl;
		}
	}

	// Try to read body (if any)
	DWORD bytesAvailable = 0;
	if (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0) {
		std::vector<char> buffer(bytesAvailable + 1);
		DWORD bytesRead = 0;
		if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead)) {
			buffer[bytesRead] = '\0';
			std::cout << "--- Response Body ---\n" << buffer.data() << std::endl;
		}
		else {
			std::cout << "[!] Failed to read body. Error: " << GetLastError() << std::endl;
		}
	}
	else {
		std::cout << "[i] No response body available or query failed." << std::endl;
	}
}
void ConnectWebSocket(const std::wstring& fullUrl, const std::wstring& accessToken)
{
	URL_COMPONENTS urlComp = { sizeof(urlComp) };
	wchar_t hostName[256], urlPath[4096];
	urlComp.lpszHostName = hostName;
	urlComp.dwHostNameLength = _countof(hostName);
	urlComp.lpszUrlPath = urlPath;
	urlComp.dwUrlPathLength = _countof(urlPath);
	if (!WinHttpCrackUrl(fullUrl.c_str(), 0, 0, &urlComp)) {
		std::wcerr << L"WinHttpCrackUrl failed: " << GetLastError() << L"\n";
		return;
	}

	HINTERNET hSession = WinHttpOpen(L"SignalRClient", WINHTTP_ACCESS_TYPE_NO_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) {
		std::wcerr << L"WinHttpOpen failed: " << GetLastError() << L"\n";
		return;
	}

	DWORD secureProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
	WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &secureProtocols, sizeof(secureProtocols));
	WinHttpSetTimeouts(hSession, 5000, 10000, 10000, 30000);

	HINTERNET hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (!hConnect) {
		std::wcerr << L"WinHttpConnect failed: " << GetLastError() << L"\n";
		WinHttpCloseHandle(hSession);
		return;
	}

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath, NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if (!hRequest) {
		std::wcerr << L"WinHttpOpenRequest failed: " << GetLastError() << L"\n";
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}

	// WebSocket headers
	std::wstring secWebSocketKey = ConvertToWide(generateRandomSecWebSocketKey());
	std::wstring headers =
		L"Connection: Upgrade\r\n"
		L"Upgrade: websocket\r\n"
		L"Sec-WebSocket-Version: 13\r\n"
		L"Sec-WebSocket-Key: " + secWebSocketKey + L"\r\n"
		L"Pragma: no-cache\r\n"
		L"Cache-Control: no-cache\r\n"
		L"Origin: console\r\n";

	WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);

	// Set option to upgrade to WebSocket
	DWORD option = WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET;
	WinHttpSetOption(hRequest, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, &option, sizeof(option));

	std::wcout << L"[i] Sending WebSocket request...\n" << headers << L"\n";

	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		std::wcerr << L"[!] SendRequest failed. Error code: " << GetLastError() << L"\n";
		DumpWinHttpErrorResponse(hRequest);
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}

	if (!WinHttpReceiveResponse(hRequest, 0)) {
		std::wcerr << L"[!] ReceiveResponse failed. Error code: " << GetLastError() << L"\n";
		DumpWinHttpErrorResponse(hRequest);
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}

	HINTERNET hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, 0);
	if (!hWebSocket) {
		std::wcerr << L"[!] WebSocket upgrade failed. Error code: " << GetLastError() << L"\n";
		DumpWinHttpErrorResponse(hRequest);
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}

	std::wcout << L"[+] WebSocket connection established.\n";

	const char* handshake = "{\"protocol\":\"json\",\"version\":1}\x1e"; // Signalr needs the \x1e for line separation of json body
	WinHttpWebSocketSend(hWebSocket, WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE, (PVOID)handshake, strlen(handshake));

	WinHttpWebSocketClose(hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);

	WinHttpCloseHandle(hWebSocket);
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
}

void TestWebSocketEcho()
{
	LPCWSTR hostname = L"ws.postman-echo.com";
	INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT;

	HINTERNET hSession = WinHttpOpen(L"WebSocket Test/1.0",
		WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	if (!hSession) {
		std::wcerr << L"WinHttpOpen failed\n";
		return;
	}

	HINTERNET hConnect = WinHttpConnect(hSession, hostname, port, 0);
	if (!hConnect) {
		std::wcerr << L"WinHttpConnect failed\n";
		WinHttpCloseHandle(hSession);
		return;
	}

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/raw",
		NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		0);

	if (!hRequest) {
		std::wcerr << L"WinHttpOpenRequest failed\n";
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}

	LPCWSTR headers = L"Connection: Upgrade\r\n"
		L"Upgrade: websocket\r\n"
		L"Sec-WebSocket-Version: 13\r\n"
		L"Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n"
		L"Origin: http://localhost\r\n";

	std::wstring key = ConvertToWide(generateRandomSecWebSocketKey());
	std::wstring fullHeaders = headers;
	fullHeaders += L"Sec-WebSocket-Key: " + key + L"\r\n";

	if (!WinHttpSendRequest(hRequest, fullHeaders.c_str(), (DWORD)-1,
		WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		std::wcerr << L"WinHttpSendRequest failed\n";
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
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
	DWORD statusCode = 0;
	DWORD statusCodeSize = sizeof(statusCode);

	// FIXED: Query the response status code correctly
	if (!WinHttpQueryHeaders(
		hRequest,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		NULL,
		&statusCode,
		&statusCodeSize,
		WINHTTP_NO_HEADER_INDEX)) {

		DWORD err = GetLastError();
		std::wcerr << L"WinHttpQueryHeaders failed: " << err << L"\n";
	}
	else {
		std::wcout << L"Status code: " << statusCode << L"\n";
	}
	if (statusCode != 101) {
		std::wcerr << L"Server did not accept WebSocket upgrade. Status code: " << statusCode << L"\n";
		// Handle error
	}

	HINTERNET hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, 0);
	if (!hWebSocket) {
		std::wcerr << L"WebSocket upgrade failed: " << GetLastError() << L"\n";
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}

	std::wcout << L"WebSocket connection established.\n";

	const char* msg = "Hello, WebSocket!";
	DWORD written = 0;
	if (WinHttpWebSocketSend(hWebSocket, WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
		(PVOID)msg, (DWORD)strlen(msg)) != NO_ERROR) {
		std::wcerr << L"Failed to send message\n";
	}
	else {
		std::wcout << L"Message sent.\n";
	}

	BYTE recvBuffer[1024];
	DWORD bytesRead = 0;
	WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
	if (WinHttpWebSocketReceive(hWebSocket, recvBuffer, sizeof(recvBuffer), &bytesRead, &bufferType) == NO_ERROR) {
		std::string reply((char*)recvBuffer, bytesRead);
		std::wcout << L"Received: " << std::wstring(reply.begin(), reply.end()) << L"\n";
	}
	else {
		std::wcerr << L"Failed to receive message\n";
	}

	WinHttpWebSocketClose(hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
	WinHttpCloseHandle(hWebSocket);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

}


int main()
{
	std::wstring deviceToken = L"[Your device token here]"; // Replace with your actual device token

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


	// usefule for testing raw websocket connection against an echo server
	//TestWebSocketEcho();

	ConnectWebSocket(finalWebSocketUrl, accessToken);

	return 0;
}
