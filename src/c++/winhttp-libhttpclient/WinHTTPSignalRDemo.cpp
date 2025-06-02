
#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include <vector>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <random>
#include <cstdlib>
#include <ctime>
#include <httpClient/httpClient.h>
#include <atomic>
#include <assert.h>
#pragma comment(lib, "winhttp.lib")

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


class win32_handle
{
public:
	win32_handle() : m_handle(nullptr)
	{
	}

	~win32_handle()
	{
		if (m_handle != nullptr) CloseHandle(m_handle);
		m_handle = nullptr;
	}

	void set(HANDLE handle)
	{
		m_handle = handle;
	}

	HANDLE get() { return m_handle; }

private:
	HANDLE m_handle;
};

win32_handle g_stopRequestedHandle;
win32_handle g_workReadyHandle;
win32_handle g_completionReadyHandle;
win32_handle g_exampleTaskDone;

DWORD g_targetNumThreads = 2;
HANDLE g_hActiveThreads[10] = { 0 };
DWORD g_defaultIdealProcessor = 0;
DWORD g_numActiveThreads = 0;

XTaskQueueHandle g_queue;
XTaskQueueRegistrationToken g_callbackToken;

DWORD WINAPI background_thread_proc(LPVOID lpParam)
{
	HANDLE hEvents[3] =
	{
		g_workReadyHandle.get(),
		g_completionReadyHandle.get(),
		g_stopRequestedHandle.get()
	};

	XTaskQueueHandle queue;
	XTaskQueueDuplicateHandle(g_queue, &queue);
	HCTraceSetTraceToDebugger(true);
	HCSettingsSetTraceLevel(HCTraceLevel::Verbose);

	bool stop = false;
	while (!stop)
	{
		DWORD dwResult = WaitForMultipleObjectsEx(3, hEvents, false, INFINITE, false);
		switch (dwResult)
		{
		case WAIT_OBJECT_0: // work ready
			if (XTaskQueueDispatch(queue, XTaskQueuePort::Work, 0))
			{
				// If we executed work, set our event again to check next time.
				SetEvent(g_workReadyHandle.get());
			}
			break;

		case WAIT_OBJECT_0 + 1: // completed 
			// Typically completions should be dispatched on the game thread, but
			// for this simple XAML app we're doing it here
			if (XTaskQueueDispatch(queue, XTaskQueuePort::Completion, 0))
			{
				// If we executed a completion set our event again to check next time
				SetEvent(g_completionReadyHandle.get());
			}
			break;

		default:
			stop = true;
			break;
		}
	}

	XTaskQueueCloseHandle(queue);
	return 0;
}

void CALLBACK HandleAsyncQueueCallback(
	_In_ void* context,
	_In_ XTaskQueueHandle queue,
	_In_ XTaskQueuePort type
)
{
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(queue);

	switch (type)
	{
	case XTaskQueuePort::Work:
		SetEvent(g_workReadyHandle.get());
		break;

	case XTaskQueuePort::Completion:
		SetEvent(g_completionReadyHandle.get());
		break;
	}
}

void StartBackgroundThread()
{
	g_stopRequestedHandle.set(CreateEvent(nullptr, true, false, nullptr));
	g_workReadyHandle.set(CreateEvent(nullptr, false, false, nullptr));
	g_completionReadyHandle.set(CreateEvent(nullptr, false, false, nullptr));
	g_exampleTaskDone.set(CreateEvent(nullptr, false, false, nullptr));

	for (uint32_t i = 0; i < g_targetNumThreads; i++)
	{
		g_hActiveThreads[i] = CreateThread(nullptr, 0, background_thread_proc, nullptr, 0, nullptr);
		if (g_defaultIdealProcessor != MAXIMUM_PROCESSORS)
		{
			if (g_hActiveThreads[i] != nullptr)
			{
				SetThreadIdealProcessor(g_hActiveThreads[i], g_defaultIdealProcessor);
			}
		}
	}

	g_numActiveThreads = g_targetNumThreads;
}

void ShutdownActiveThreads()
{
	SetEvent(g_stopRequestedHandle.get());
	DWORD dwResult = WaitForMultipleObjectsEx(g_numActiveThreads, g_hActiveThreads, true, INFINITE, false);
	if (dwResult >= WAIT_OBJECT_0 && dwResult <= WAIT_OBJECT_0 + g_numActiveThreads - 1)
	{
		for (DWORD i = 0; i < g_numActiveThreads; i++)
		{
			CloseHandle(g_hActiveThreads[i]);
			g_hActiveThreads[i] = nullptr;
		}
		g_numActiveThreads = 0;
		ResetEvent(g_stopRequestedHandle.get());
	}
}

struct WebSocketContext
{
	WebSocketContext()
	{
		closeEventHandle = CreateEvent(nullptr, false, false, nullptr);
	}

	~WebSocketContext()
	{
		if (handle)
		{
			HCWebSocketCloseHandle(handle);
		}
		CloseHandle(closeEventHandle);
	}

	HCWebsocketHandle handle{ nullptr };
	std::vector<char> receiveBuffer;
	std::atomic<uint32_t> messagesReceived;
	HANDLE closeEventHandle;
};

void CALLBACK WebSocketMessageReceived(
	_In_ HCWebsocketHandle websocket,
	_In_z_ const char* incomingBodyString,
	_In_opt_ void* functionContext
)
{
	auto ctx = static_cast<WebSocketContext*>(functionContext);

	printf_s("Received websocket message: %s\n", incomingBodyString);
	++ctx->messagesReceived;
}

void CALLBACK WebSocketBinaryMessageReceived(
	_In_ HCWebsocketHandle websocket,
	_In_reads_bytes_(payloadSize) const uint8_t* payloadBytes,
	_In_ uint32_t payloadSize,
	_In_ void* functionContext
)
{
	auto ctx = static_cast<WebSocketContext*>(functionContext);

	printf_s("Received websocket binary message of size: %u\r\n", payloadSize);

	++ctx->messagesReceived;
}

void CALLBACK WebSocketBinaryMessageFragmentReceived(
	_In_ HCWebsocketHandle websocket,
	_In_reads_bytes_(payloadSize) const uint8_t* payloadBytes,
	_In_ uint32_t payloadSize,
	_In_ bool isFinalFragment,
	_In_ void* functionContext
)
{
	auto ctx = static_cast<WebSocketContext*>(functionContext);

	printf("Received websocket binary message fragment of size %u\r\n", payloadSize);
	ctx->receiveBuffer.insert(ctx->receiveBuffer.end(), payloadBytes, payloadBytes + payloadSize);
	if (isFinalFragment)
	{
		printf_s("Full message now received: %s\n", ctx->receiveBuffer.data());
		++ctx->messagesReceived;
		ctx->receiveBuffer.clear();
	}
}

void CALLBACK WebSocketClosed(
	_In_ HCWebsocketHandle websocket,
	_In_ HCWebSocketCloseStatus closeStatus,
	_In_opt_ void* functionContext
)
{
	auto ctx = static_cast<WebSocketContext*>(functionContext);

	printf_s("Websocket closed!\n");
	SetEvent(ctx->closeEventHandle);
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

	HCInitialize(nullptr);
	HCSettingsSetTraceLevel(HCTraceLevel::Verbose);

	XTaskQueueCreate(XTaskQueueDispatchMode::Manual, XTaskQueueDispatchMode::Manual, &g_queue);
	XTaskQueueRegisterMonitor(g_queue, nullptr, HandleAsyncQueueCallback, &g_callbackToken);
	StartBackgroundThread();
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string url = converter.to_bytes(fullUrl);

	auto websocketContext = new WebSocketContext{};
	HCWebSocketCreate(&websocketContext->handle, WebSocketMessageReceived, WebSocketBinaryMessageReceived, WebSocketClosed, websocketContext);
	HCWebSocketSetBinaryMessageFragmentEventFunction(websocketContext->handle, WebSocketBinaryMessageFragmentReceived);
	HCWebSocketSetMaxReceiveBufferSize(websocketContext->handle, 4096);

	XAsyncBlock* asyncBlock = new XAsyncBlock{};
	asyncBlock->queue = g_queue;
	asyncBlock->callback = [](XAsyncBlock* asyncBlock)
		{
			WebSocketCompletionResult result = {};
			HRESULT hr = HCGetWebSocketConnectResult(asyncBlock, &result);
			assert(SUCCEEDED(hr));

			if (SUCCEEDED(hr))
			{
				printf_s("HCWebSocketConnect complete: %d, %d\n", result.errorCode, result.platformErrorCode);
				if (FAILED(result.errorCode))
				{
					throw std::exception("Connect failed. Make sure a local echo server is running.");
				}
			}

			delete asyncBlock;
		};

	printf_s("Calling HCWebSocketConnect...\n");
	HCWebSocketConnectAsync(url.data(), "", websocketContext->handle, asyncBlock);
	XAsyncGetStatus(asyncBlock, true);
	// Send the SignalR handshake message
	const char* handshakeMsg = "{\"protocol\":\"json\",\"version\":1}\x1e"; // SignalR handshake with JSON protocol
	
	asyncBlock = new XAsyncBlock{};
	asyncBlock->queue = g_queue;
	asyncBlock->callback = [](XAsyncBlock* asyncBlock)
		{
			WebSocketCompletionResult result = {};
			HRESULT hr = HCGetWebSocketSendMessageResult(asyncBlock, &result);
			assert(SUCCEEDED(hr));

			if (SUCCEEDED(hr))
			{
				printf_s("SignalR handshake sent: %d, %d\n", result.errorCode, result.platformErrorCode);
			}
			delete asyncBlock;
		};

	printf_s("Sending SignalR handshake message: %s\n", handshakeMsg);
	HCWebSocketSendMessageAsync(websocketContext->handle, handshakeMsg, asyncBlock);

	// Wait for handshake response
	printf_s("Waiting for SignalR handshake response...\n");
	
	// Wait a reasonable amount of time for the server to respond to our handshake
	// The standard wait loop just monitors messagesReceived count
	const uint32_t maxWaitMs = 5000; // 5 seconds timeout
	uint32_t waitedMs = 0;
	const uint32_t sleepIntervalMs = 100;
	
	while (websocketContext->messagesReceived < 1 && waitedMs < maxWaitMs)
	{
		Sleep(sleepIntervalMs);
		waitedMs += sleepIntervalMs;
	}
	
	if (websocketContext->messagesReceived < 1)
	{
		printf_s("Warning: No handshake response received within timeout.\n");
	}
	else
	{
		printf_s("SignalR handshake completed successfully.\n");
		
		// Now you can send additional messages if needed
		// For example, you could join a hub or invoke a method
	}

	printf_s("Calling HCWebSocketDisconnect...\n");
	HCWebSocketDisconnect(websocketContext->handle);
	WaitForSingleObject(websocketContext->closeEventHandle, INFINITE);
	delete websocketContext;

	HCCleanup();
}

int main()
{
	std::wstring deviceToken = L"YTRmYjg5NDItMTVkMS00OTc5LWE3YjYtYTY3MjQyMTJiOTZhfHx8VkRnKUBTKihjbzNyZE1DSnpVQzNGMzZlZ19kN0hK"; // Replace with your actual device token

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
