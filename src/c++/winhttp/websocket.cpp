#include <stdio.h>
#include <Windows.h>
#include <winhttp.h>
#include <wincrypt.h>
#include <iostream>
#include <string>
#include <cctype>

#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Crypt32.lib")

#include "websocket.h"
using namespace WinHttpWebSocketClient;

void WinHttpWebSocketClient::PrintLastError(DWORD errorCode, WCHAR* des, size_t desLen, const std::wstring action, bool append)
{
	size_t offset;

	if (!append) {
		offset = 0;
	}
	else {
		offset = wcslen(des);
	}

	if (offset > desLen) { // Buffer too small!
		return;
	}

	if (errorCode == ERROR_INVALID_OPERATION) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_INVALID_OPERATION");
	}
	else if (errorCode == ERROR_WINHTTP_CANNOT_CONNECT) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_CANNOT_CONNECT");
	}
	else if (errorCode == ERROR_WINHTTP_INVALID_SERVER_RESPONSE) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_INVALID_SERVER_RESPONSE");
	}
	else if (errorCode == ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED");
	}
	else if (errorCode == ERROR_WINHTTP_CONNECTION_ERROR) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_CONNECTION_ERROR");
	}
	else if (errorCode == ERROR_WINHTTP_INCORRECT_HANDLE_STATE) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_INCORRECT_HANDLE_STATE");
	}
	else if (errorCode == ERROR_WINHTTP_INCORRECT_HANDLE_TYPE) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_INCORRECT_HANDLE_TYPE");
	}
	else if (errorCode == ERROR_WINHTTP_INTERNAL_ERROR) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_INTERNAL_ERROR");
	}
	else if (errorCode == ERROR_WINHTTP_INVALID_URL) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_INVALID_URL");
	}
	else if (errorCode == ERROR_WINHTTP_LOGIN_FAILURE) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_LOGIN_FAILURE");
	}
	else if (errorCode == ERROR_WINHTTP_NAME_NOT_RESOLVED) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_NAME_NOT_RESOLVED");
	}
	else if (errorCode == ERROR_WINHTTP_OPERATION_CANCELLED) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_OPERATION_CANCELLED");
	}
	else if (errorCode == ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW");
	}
	else if (errorCode == ERROR_WINHTTP_SECURE_FAILURE) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_SECURE_FAILURE");
	}
	else if (errorCode == ERROR_WINHTTP_SHUTDOWN) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_SHUTDOWN");
	}
	else if (errorCode == ERROR_WINHTTP_TIMEOUT) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_TIMEOUT");
	}
	else if (errorCode == ERROR_WINHTTP_UNRECOGNIZED_SCHEME) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_UNRECOGNIZED_SCHEME");
	}
	else if (errorCode == ERROR_NOT_ENOUGH_MEMORY) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_NOT_ENOUGH_MEMORY");
	}
	else if (errorCode == ERROR_INVALID_PARAMETER) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_INVALID_PARAMETER");
	}
	else if (errorCode == ERROR_WINHTTP_RESEND_REQUEST) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_RESEND_REQUEST");
	}
	else if (errorCode == ERROR_WINHTTP_INVALID_OPTION) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_INVALID_OPTION");
	}
	else if (errorCode == ERROR_WINHTTP_OPTION_NOT_SETTABLE) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_WINHTTP_OPTION_NOT_SETTABLE");
	}
	else if (errorCode == ERROR_INSUFFICIENT_BUFFER) {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls", action.c_str(), L"failed; ERROR_INSUFFICIENT_BUFFER");
	}
	else {
		swprintf_s(&des[offset], desLen - offset, L"%ls %ls 0x%lX", action.c_str(), L"failed; error code =", errorCode);
	}
}

void SecureFailureCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	WebSocketClient* webSocket = (WebSocketClient*)dwContext;
	if (webSocket != NULL)
	{
		if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_SECURE_FAILURE)
		{
			if (*((DWORD*)lpvStatusInformation) == WINHTTP_CALLBACK_STATUS_FLAG_CERT_REV_FAILED) {
				swprintf_s(webSocket->ErrorDescription, webSocket->ErrorBufferLength, L"%ls", L"WinHTTP Secure Failure error: WINHTTP_CALLBACK_STATUS_FLAG_CERT_REV_FAILED\n");
			}
			else if (*((DWORD*)lpvStatusInformation) == WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CERT) {
				swprintf_s(webSocket->ErrorDescription, webSocket->ErrorBufferLength, L"%ls", L"WinHTTP Secure Failure error: WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CERT\n");
			}
			else if (*((DWORD*)lpvStatusInformation) == WINHTTP_CALLBACK_STATUS_FLAG_CERT_REVOKED) {
				swprintf_s(webSocket->ErrorDescription, webSocket->ErrorBufferLength, L"%ls", L"WinHTTP Secure Failure error: WINHTTP_CALLBACK_STATUS_FLAG_CERT_REVOKED\n");
			}
			else if (*((DWORD*)lpvStatusInformation) == WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CA) {
				swprintf_s(webSocket->ErrorDescription, webSocket->ErrorBufferLength, L"%ls", L"WinHTTP Secure Failure error: WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CA\n");
			}
			else if (*((DWORD*)lpvStatusInformation) == WINHTTP_CALLBACK_STATUS_FLAG_CERT_CN_INVALID) {
				swprintf_s(webSocket->ErrorDescription, webSocket->ErrorBufferLength, L"%ls", L"WinHTTP Secure Failure error: WINHTTP_CALLBACK_STATUS_FLAG_CERT_CN_INVALID\n");
			}
			else if (*((DWORD*)lpvStatusInformation) == WINHTTP_CALLBACK_STATUS_FLAG_CERT_DATE_INVALID) {
				swprintf_s(webSocket->ErrorDescription, webSocket->ErrorBufferLength, L"%ls", L"WinHTTP Secure Failure error: WINHTTP_CALLBACK_STATUS_FLAG_CERT_DATE_INVALID\n");
			}
			else if (*((DWORD*)lpvStatusInformation) == WINHTTP_CALLBACK_STATUS_FLAG_SECURITY_CHANNEL_ERROR) {
				swprintf_s(webSocket->ErrorDescription, webSocket->ErrorBufferLength, L"%ls", L"WinHTTP Secure Failure error: WINHTTP_CALLBACK_STATUS_FLAG_SECURITY_CHANNEL_ERROR\n");
			}
			else if (*((DWORD*)lpvStatusInformation) == ERROR_BAD_LENGTH) {
				swprintf_s(webSocket->ErrorDescription, webSocket->ErrorBufferLength, L"%ls", L"WinHTTP Secure Failure error: ERROR_BAD_LENGTH\n");
			}
			else {
				swprintf_s(webSocket->ErrorDescription, webSocket->ErrorBufferLength, L"%ls 0x%lX\n", L"WinHTTP Secure Failure error:", *((DWORD*)lpvStatusInformation));
			}
		}
	}
}

PCCERT_CONTEXT WinHttpWebSocketClient::OpenCertificateByName(WCHAR* subjectName, WCHAR* store, bool userStores)
{
	HCERTSTORE hStore;
	PCCERT_CONTEXT pCertContext;
	DWORD dwFlags = 0;

	// Return NULL if we can't find the certificate
	pCertContext = NULL;

	dwFlags |= CERT_STORE_READONLY_FLAG;
	if (userStores) {
		dwFlags |= CERT_SYSTEM_STORE_CURRENT_USER;
	}
	else {
		dwFlags |= CERT_SYSTEM_STORE_LOCAL_MACHINE;
	}

	if (store != NULL)
	{
		hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, dwFlags, store);

		if (hStore)
		{
			pCertContext = CertFindCertificateInStore(hStore,
				X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0,
				CERT_FIND_SUBJECT_STR, (LPVOID)subjectName, NULL);

			CertCloseStore(hStore, 0);
		}
	}
	else
	{
		const WCHAR* stores[] = { L"My", L"WebHosting" };

		int i = 0;
		while ((pCertContext == NULL) && (i < ARRAYSIZE(stores)))
		{
			hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, dwFlags, stores[i]);

			if (hStore)
			{
				pCertContext = CertFindCertificateInStore(hStore,
					X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0,
					CERT_FIND_SUBJECT_STR, (LPVOID)subjectName, NULL);

				CertCloseStore(hStore, 0);
			}

			i++;
		}
	}

	return pCertContext;
}

DWORD WebSocketClient::Initialize(HINTERNET hSession, PCCERT_CONTEXT pCertContext)
{
	// Set class data to zero
	memset(this, 0, sizeof(WebSocketClient));

	// Set hSession
	this->hSession = hSession;

	// Set certificate to use for connections
	this->pCertContext = pCertContext;

	// Set the length of the description buffer
	this->ErrorBufferLength = 0x1000;

	// Allocate memory for the description buffer
	this->ErrorDescription = (WCHAR*)malloc(sizeof(WCHAR) * this->ErrorBufferLength);
	if (this->ErrorDescription == NULL) {
		this->ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
	}

	// Return error code
	return this->ErrorCode;
}

VOID WebSocketClient::Free()
{
	// Free resources
	if (this->ErrorDescription) {
		free(this->ErrorDescription);
	}

	// Set class data to zero
	memset(this, 0, sizeof(WebSocketClient));
}

DWORD WebSocketClient::Close(WINHTTP_WEB_SOCKET_CLOSE_STATUS status, CHAR* reason)
{
	// Length of reason in bytes
	DWORD reasonLen;
	if (reason == NULL) {
		reasonLen = 0;
	}
	else {
		reasonLen = (DWORD)strlen(reason);
	}

	// Gracefully close the connection
	this->ErrorCode = WinHttpWebSocketClose(this->hWebSocket, status, reason, reasonLen);

	// Close the connection handles
	WinHttpCloseHandle(this->hWebSocket);
	WinHttpCloseHandle(this->hRequest);
	WinHttpCloseHandle(this->hConnect);

	// Set class variables to NULL
	this->hWebSocket = NULL;
	this->hRequest = NULL;
	this->hConnect = NULL;

	// Return error code
	return this->ErrorCode;
}

DWORD WebSocketClient::Connect(const std::wstring host, DWORD flags, WCHAR* protocol)
{


	DWORD dwFlags = 0;
	// Return 0 for success
	DWORD errorCode = ERROR_SUCCESS;

	// Cracked URL variable pointers
	URL_COMPONENTS UrlComponents;

	// Create cracked URL buffer variables
	WCHAR* scheme = NULL;
	WCHAR* hostName = NULL;
	WCHAR* urlPath = NULL;

	// Sec-WebSocket-Protocol header string buffer
	WCHAR* websocket_protocol = NULL;

	// Option variables for WinHttpSetOption
	void* ptrOptionValue;

	// Allocate buffers

	// Max of 32 characters
	scheme = (WCHAR*)malloc(sizeof(WCHAR) * 0x20);
	if (scheme == NULL) {
		errorCode = ERROR_NOT_ENOUGH_MEMORY;
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WebSocketClient::Connect()");
		goto errorExit;
	}
	// Max of 256 characters
	hostName = (WCHAR*)malloc(sizeof(WCHAR) * 0x100);
	if (hostName == NULL) {
		errorCode = ERROR_NOT_ENOUGH_MEMORY;
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WebSocketClient::Connect()");
		goto errorExit;
	}
	// Max of 4096 characters
	urlPath = (WCHAR*)malloc(sizeof(WCHAR) * 0x1000);
	if (urlPath == NULL) {
		errorCode = ERROR_NOT_ENOUGH_MEMORY;
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WebSocketClient::Connect()");
		goto errorExit;
	}


	std::cout << urlPath << std::endl;

	// Clear error's
	this->ErrorCode = 0;
	this->ErrorDescription[0] = NULL;

	// Clear old/failed handles
	this->hConnect = NULL;
	this->hRequest = NULL;
	this->hWebSocket = NULL;

	// Setup UrlComponents structure
	memset(&UrlComponents, 0, sizeof(URL_COMPONENTS));
	UrlComponents.dwStructSize = sizeof(URL_COMPONENTS);
	UrlComponents.dwSchemeLength = -1;
	UrlComponents.dwHostNameLength = -1;
	UrlComponents.dwUserNameLength = -1;
	UrlComponents.dwPasswordLength = -1;
	UrlComponents.dwUrlPathLength = -1;
	UrlComponents.dwExtraInfoLength = -1;

	// Get the individual parts of the url
	if (!WinHttpCrackUrl(host.c_str(), NULL, 0, &UrlComponents))
	{
		// Handle error
		errorCode = GetLastError();
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WinHttpCrackUrl()", true);
		goto errorExit;
	}

	// Copy cracked URL hostName & UrlPath to buffers so they are separated
	wcsncpy_s(scheme, 0x20, UrlComponents.lpszScheme, UrlComponents.dwSchemeLength);
	wcsncpy_s(hostName, 0x100, UrlComponents.lpszHostName, UrlComponents.dwHostNameLength);
	wcsncpy_s(urlPath, 0x1000, UrlComponents.lpszUrlPath, UrlComponents.dwUrlPathLength);

	std::cout << urlPath << std::endl;


	if (UrlComponents.nPort == 0) {
		if ((_wcsicmp(scheme, L"wss") == 0) || (_wcsicmp(scheme, L"https") == 0)) {
			UrlComponents.nPort = INTERNET_DEFAULT_HTTPS_PORT;
		}
		else if ((_wcsicmp(scheme, L"ws") == 0) || (_wcsicmp(scheme, L"http")) == 0) {
			UrlComponents.nPort = INTERNET_DEFAULT_HTTP_PORT;
		}
		else {
			errorCode = ERROR_INVALID_PARAMETER;
			PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WebSocketClient::Connect()", true);
			goto errorExit;
		}
	}

	// Call the WinHttp Connect method
	this->hConnect = WinHttpConnect(this->hSession, hostName, UrlComponents.nPort, 0);
	if (!this->hConnect)
	{
		// Handle error
		errorCode = GetLastError();
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WinHttpConnect()", true);
		goto errorExit;
	}

	if (flags & WEBSOCKET_SECURE_CONNECTION) {
		dwFlags |= WINHTTP_FLAG_SECURE;
	}

	// Create a HTTP request
	this->hRequest = WinHttpOpenRequest(this->hConnect, L"GET", urlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, dwFlags);
	if (!this->hRequest)
	{
		// Handle error
		errorCode = GetLastError();
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WinHttpOpenRequest()", true);
		goto errorExit;
	}

	// Set option for client certificate
	if (this->pCertContext == NULL) {
		if (!WinHttpSetOption(this->hRequest, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, WINHTTP_NO_CLIENT_CERT_CONTEXT, 0))
		{
			// Handle error
			errorCode = GetLastError();
			PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WinHttpSetOption()", true);
			goto errorExit;
		}
	}
	else {
		ptrOptionValue = (void*)this->pCertContext;
		if (!WinHttpSetOption(this->hRequest, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, (LPVOID)this->pCertContext, sizeof(CERT_CONTEXT)))
		{
			// Handle error
			errorCode = GetLastError();
			PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WinHttpSetOption()", true);
			goto errorExit;
		}
	}

	// Set the request context to our websocket
	ptrOptionValue = this;
	if (!WinHttpSetOption(this->hRequest, WINHTTP_OPTION_CONTEXT_VALUE, &ptrOptionValue, sizeof(ptrOptionValue)))
	{
		// Handle error
		errorCode = GetLastError();
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WinHttpSetStatusCallback()", true);
		goto errorExit;
	}

	// Set callback function for debugging secure failures
	if (WinHttpSetStatusCallback(this->hRequest, SecureFailureCallback, WINHTTP_CALLBACK_FLAG_SECURE_FAILURE, 0) == WINHTTP_INVALID_STATUS_CALLBACK)
	{
		// Handle error
		errorCode = GetLastError();
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WinHttpSetStatusCallback()", true);
		goto errorExit;
	}

	// Add WebSocket upgrade to our HTTP request
#pragma prefast(suppress:6387, "WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET does not take any arguments.")
	if (!WinHttpSetOption(this->hRequest, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, 0, 0))
	{
		// Handle error
		errorCode = GetLastError();
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WinHttpSetOption()", true);
		goto errorExit;
	}

	if (protocol != NULL)
	{
		// Allocate temporary memory for header string
		size_t headerLen = wcslen(protocol) + 0x100;
		websocket_protocol = (WCHAR*)malloc(sizeof(WCHAR) * headerLen);

		// Handle error
		if (websocket_protocol == NULL) {
			errorCode = ERROR_NOT_ENOUGH_MEMORY;
			PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"Allocating memory for request header", true);
			goto errorExit;
		}

		// Build the "Sec-WebSocket-Protocol" header
		swprintf_s(websocket_protocol, headerLen, L"%ls %ls", L"Sec-WebSocket-Protocol:", protocol);

		// Add to the request handle
		WinHttpAddRequestHeaders(this->hRequest, websocket_protocol, -1L, WINHTTP_ADDREQ_FLAG_ADD);
	}

	// Send the WebSocket upgrade request.
	if (!WinHttpSendRequest(this->hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, 0, 0, 0, 0))
	{
		// Handle error
		errorCode = GetLastError();
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WinHttpSendRequest()", true);
		goto errorExit;
	}

	// Receive response from the server
	if (!WinHttpReceiveResponse(this->hRequest, 0))
	{
		// Handle error
		errorCode = GetLastError();
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WinHttpReceiveResponse()", true);
		goto errorExit;
	}

	// Finally complete the upgrade
	this->hWebSocket = WinHttpWebSocketCompleteUpgrade(this->hRequest, (DWORD_PTR)this);
	if (this->hWebSocket == 0)
	{
		// Handle error
		errorCode = GetLastError();
		PrintLastError(errorCode, this->ErrorDescription, this->ErrorBufferLength, L"WinHttpWebSocketCompleteUpgrade()", true);
		goto errorExit;
	}

	// Free resources
	if (scheme) free(scheme);
	if (hostName) free(hostName);
	if (urlPath) free(urlPath);
	if (websocket_protocol) free(websocket_protocol);

	// Set error code
	this->ErrorCode = errorCode;

	// Return should be zero
	return errorCode;

errorExit:

	// Close handles
	if (this->hWebSocket) WinHttpCloseHandle(this->hWebSocket);
	if (this->hRequest) WinHttpCloseHandle(this->hRequest);
	if (this->hConnect) WinHttpCloseHandle(this->hConnect);

	// Free resources
	if (scheme) free(scheme);
	if (hostName) free(hostName);
	if (urlPath) free(urlPath);
	if (websocket_protocol) free(websocket_protocol);

	// Set error code
	this->ErrorCode = errorCode;

	// Return error code
	return errorCode;
}

DWORD WebSocketClient::Receive(void* pBuffer, DWORD dwBufferLength, DWORD* pdwBytesReceived, WINHTTP_WEB_SOCKET_BUFFER_TYPE* pBufferType)
{
	// Read data from the server
	this->ErrorCode = WinHttpWebSocketReceive(this->hWebSocket, pBuffer, dwBufferLength, pdwBytesReceived, pBufferType);

	// Return error code
	return this->ErrorCode;
}

DWORD WebSocketClient::Send(WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType, void* pBuffer, DWORD dwLength)
{
	// Send the data to the server
	this->ErrorCode = WinHttpWebSocketSend(this->hWebSocket, bufferType, pBuffer, dwLength);

	// Return error code
	return this->ErrorCode;
}

DWORD WebSocketClient::QueryCloseStatus(USHORT* pusStatus, PVOID pvReason, DWORD dwReasonLength, DWORD* pdwReasonLengthConsumed)
{
	// Query the close status
	this->ErrorCode = WinHttpWebSocketQueryCloseStatus(this->hWebSocket, pusStatus, pvReason, dwReasonLength, pdwReasonLengthConsumed);

	// Return error code
	return this->ErrorCode;
}