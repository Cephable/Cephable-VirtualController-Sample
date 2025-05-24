#ifndef WINHTTP_WEB_SOCKET_CLIENT_H
#define WINHTTP_WEB_SOCKET_CLIENT_H

#include <Windows.h>
#include <winhttp.h>

// Connect flags

#define WEBSOCKET_SECURE_CONNECTION        0x0001

// WinHTTP WebSocket client namespace
namespace WinHttpWebSocketClient
{
	// Print a Windows Error Code
	void PrintLastError(DWORD errorCode, WCHAR* des, size_t desLen, std::wstring, bool append = false);

	// Open a certificate from a Windows certificate store
	// - If store is NULL then the function searches "My" and "WebHosting" stores
	PCCERT_CONTEXT OpenCertificateByName(WCHAR* subjectName, WCHAR* store, bool userStores);

	// WebSocket client class
	class WebSocketClient {
	private:
		// Application session handle to use with this connection
		HINTERNET hSession;
		// Windows connect handle
		HINTERNET hConnect;
		// The initial HTTP request handle to start the WebSocket handshake
		HINTERNET hRequest;
		// Windows WebSocket handle
		HINTERNET hWebSocket;
		// The client certificate used for the connection
		PCCERT_CONTEXT pCertContext;
	public:
		// Error of the called function
		DWORD ErrorCode;
		// The description of the error code
		WCHAR* ErrorDescription;
		// The number of characters the ErrorDescription buffer can hold
		size_t ErrorBufferLength;
		// Initialize the WebSocket client class
		DWORD Initialize(HINTERNET hSession, PCCERT_CONTEXT pCertContext);
		// Connect to a WebSocket server
		DWORD Connect(std::wstring host, DWORD flags, WCHAR* protocol = NULL);
		// Receive data from the WebSocket server
		DWORD Receive(void* pBuffer, DWORD dwBufferLength, DWORD* pdwBytesReceived, WINHTTP_WEB_SOCKET_BUFFER_TYPE* pBufferType);
		// Send data to the WebSocket server
		DWORD Send(WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType, void* pBuffer, DWORD dwLength);
		// Close the connection to the server
		DWORD Close(WINHTTP_WEB_SOCKET_CLOSE_STATUS status, CHAR* reason = NULL);
		// Retrieve the close status sent by a server
		DWORD QueryCloseStatus(USHORT* pusStatus, PVOID pvReason, DWORD dwReasonLength, DWORD* pdwReasonLengthConsumed);
		// Free resources
		VOID Free();
	};
}

#endif // !WINHTTP_WEB_SOCKET_CLIENT_H