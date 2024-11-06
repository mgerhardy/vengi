/**
 * @file
 */

#include "Request.h"
#include "app/App.h"
#include "core/ArrayLength.h"
#include "core/ConfigVar.h"
#include "core/Log.h"
#include "core/StringUtil.h"
#include "core/Var.h"
#include "engine-config.h"

#if SDL_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN (1)
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <sstream>
#elif EMSCRIPTEN
#include <emscripten/fetch.h>
#elif USE_CURL
#include <curl/curl.h>
#endif

namespace http {

#ifdef SDL_PLATFORM_WINDOWS
static void printLastError(const char *ctx) {
	DWORD errnum = ::GetLastError();
	char buffer[512] = "";
	if (::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
						 GetModuleHandleA("winhttp.dll"), errnum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer,
						 sizeof(buffer), NULL) == 0) {
		Log::error("%s: %d - Unknown error", ctx, (int)errnum);
	} else {
		Log::error("%s: %d - %s", ctx, (int)errnum, buffer);
	}
}

static std::wstring s2ws(const std::string &str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

static std::string ws2s(const std::wstring &wstr) {
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

#elif USE_CURL
static size_t WriteHeaderData(char *b, size_t size, size_t nitems, void *userdata) {
	core::StringMap<core::String> *headers = (core::StringMap<core::String> *)userdata;
	core::String str;

	size_t total = size * nitems;
	if (total)
		str.append(b, total);

	if (str.last() == '\n')
		str.erase(str.size() - 1);
	if (str.last() == '\r')
		str.erase(str.size() - 1);

	size_t pos = str.find_first_of(':');
	if (pos != core::String::npos) {
		const core::String &key = str.substr(0, pos);
		const core::String &value = str.substr(pos + 2);
		Log::debug("Header: %s: %s", key.c_str(), value.c_str());
		headers->put(key, value);
	}
	return total;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	return ((io::WriteStream *)userp)->write(contents, size * nmemb);
}
#endif

bool Request::supported() {
#if SDL_PLATFORM_WINDOWS
	return true;
#elif USE_CURL
	return true;
#elif EMSCRIPTEN
	return true;
#else
	return false;
#endif
}

Request::Request(const core::String &url, RequestType type) : _type(type), _url(url) {
	_timeoutSecond = core::Var::get(cfg::HttpTimeout, "5")->intVal();
	_connectTimeoutSecond = core::Var::get(cfg::HttpConnectTimeout, "1")->intVal();
	_userAgent = "vengi/" PROJECT_VERSION;
}

bool Request::setBody(const core::String &body) {
	core_assert(_type == RequestType::POST);
	if (_type != RequestType::POST) {
		return false;
	}
	_body = body;
	return true;
}

void Request::addHeader(const core::String &key, const core::String &value) {
	_headers.put(key, value);
}

void Request::noCache() {
	addHeader("Cache-Control", "no-cache");
}

bool Request::execute(io::WriteStream &stream, int *statusCode, core::StringMap<core::String> *outheaders) {
	Log::debug("Starting http request for %s", _url.c_str());
#if SDL_PLATFORM_WINDOWS
	// Initialize WinHTTP and create a session
	HINTERNET hSession =
		WinHttpOpen(nullptr, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession == nullptr) {
		printLastError("Failed to create session for http download");
		return false;
	}

	// Set the connection timeout
	DWORD dwResolveTimeout = _timeoutSecond * 1000;
	DWORD dwConnectTimeout = _connectTimeoutSecond * 1000;
	DWORD dwSendTimeout = dwResolveTimeout;
	DWORD dwReceiveTimeout = dwResolveTimeout;
	if (!WinHttpSetTimeouts(hSession, dwResolveTimeout, dwConnectTimeout, dwSendTimeout, dwReceiveTimeout)) {
		printLastError("Failed to set http timeouts");
		WinHttpCloseHandle(hSession);
		return false;
	}

	std::wstring urlw = s2ws(_url.c_str());
	URL_COMPONENTS url_components = {};
	wchar_t scheme[32];
	wchar_t hostname[128];
	wchar_t url_path[4096];

	/* Convert the URL to its components. */
	url_components.dwStructSize = sizeof(url_components);
	url_components.lpszScheme = scheme;
	url_components.dwSchemeLength = lengthof(scheme);
	url_components.lpszHostName = hostname;
	url_components.dwHostNameLength = lengthof(hostname);
	url_components.lpszUrlPath = url_path;
	url_components.dwUrlPathLength = lengthof(url_path);
	url_components.nPort = INTERNET_DEFAULT_HTTP_PORT;
	if (!WinHttpCrackUrl(urlw.c_str(), 0, 0, &url_components)) {
		printLastError("Failed to parse url");
		WinHttpCloseHandle(hSession);
		return false;
	}

	/* Create the HTTP connection. */
	HINTERNET hConnection = WinHttpConnect(hSession, url_components.lpszHostName, url_components.nPort, 0);
	if (hConnection == nullptr) {
		printLastError("Failed to connect");
		WinHttpCloseHandle(hSession);
		return false;
	}

	const wchar_t *method = _type == RequestType::GET ? L"GET" : L"POST";
	HINTERNET hRequest = WinHttpOpenRequest(hConnection, method, url_components.lpszUrlPath, nullptr,
											WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
											url_components.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);
	if (hRequest == nullptr) {
		printLastError("Failed to create request");
		WinHttpCloseHandle(hSession);
		WinHttpCloseHandle(hConnection);
		return false;
	}

	// add request headers

	std::wstring reqHeaders;
	if (!_userAgent.empty()) {
		reqHeaders += L"User-Agent: ";
		reqHeaders += s2ws(_userAgent.c_str());
		reqHeaders += L"\r\n";
	}
	for (const auto &entry : _headers) {
		reqHeaders += s2ws(entry->first.c_str());
		reqHeaders += L": ";
		reqHeaders += s2ws(entry->second.c_str());
		reqHeaders += L"\r\n";
	}

	SIZE_T len = reqHeaders.size();
	if (len > 0 && !WinHttpAddRequestHeaders(hRequest, reqHeaders.c_str(), DWORD(len),
								  WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE)) {
		Log::error("Failed to add request headers to url: %s", _url.c_str());
		printLastError("Failed to add request headers");
	}

	// Send the request
	int maxRedirects = 3;
	bool requestState = false;
	if (_type == RequestType::GET) {
		while (!requestState) {
			requestState =
				WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
			if (!requestState && GetLastError() == ERROR_WINHTTP_RESEND_REQUEST) {
				if (maxRedirects <= 0) {
					break;
				}
				--maxRedirects;
				continue;
			}
			if (!requestState) {
				Log::error("Failed to send request with error %d", GetLastError());
				break;
			}
		}
	} else {
		while (!requestState) {
			requestState = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)_body.c_str(), _body.size(),
											  _body.size(), 0);
			if (!requestState && GetLastError() == ERROR_WINHTTP_RESEND_REQUEST) {
				if (maxRedirects <= 0) {
					break;
				}
				--maxRedirects;
				continue;
			}
			if (!requestState) {
				Log::error("Failed to send request with error %d", (int)GetLastError());
				break;
			}
		}
	}
	WinHttpReceiveResponse(hRequest, nullptr);
	if (!WinHttpQueryDataAvailable(hRequest, nullptr)) {
		printLastError("Failed to query available data");
	}

	DWORD dwStatusCode = 0;
	DWORD dwSize = sizeof(dwStatusCode);
	if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
						&dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX)) {
		printLastError("Failed to query status code");
	}
	const char *requestTypeStr = _type == RequestType::GET ? "GET" : "POST";
	Log::debug("Http request for url: %s (%s) with status code: %d", _url.c_str(), requestTypeStr, (int)dwStatusCode);
	if (statusCode) {
		*statusCode = (int)dwStatusCode;
	}

	if (outheaders) {
		DWORD headerLength = sizeof(DWORD);
		if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, nullptr,
								 &headerLength, WINHTTP_NO_HEADER_INDEX) &&
			GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			wchar_t *rawHeader = new wchar_t[headerLength / sizeof(wchar_t)];
			ZeroMemory(rawHeader, headerLength);
			if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, rawHeader,
									&headerLength, WINHTTP_NO_HEADER_INDEX)) {
				std::wstring headerStr(rawHeader);
				std::wstringstream headerStream(headerStr);
				std::wstring line;
				while (std::getline(headerStream, line)) {
					size_t delimiter = line.find(L": ");
					if (delimiter != std::wstring::npos) {
						const std::string &key = ws2s(line.substr(0, delimiter));
						const std::string &value = ws2s(line.substr(delimiter + 2));
						outheaders->put(key.c_str(), value.c_str());
					}
				}
			}
			delete[] rawHeader;
		}
	}

	// Read and save the response data
	DWORD bytesRead;
	BYTE buffer[4096];
	while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead)) {
		// Write the 'bytesRead' bytes from the buffer
		if (bytesRead == 0) {
			break;
		}
		stream.write(buffer, bytesRead);
	}
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hSession);
	WinHttpCloseHandle(hConnection);
	return true;
#elif EMSCRIPTEN
	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	// TODO: use EMSCRIPTEN_FETCH_STREAM_DATA
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
	const core::String method = _type == RequestType::GET ? "GET" : "POST";
	core::string::strncpyz(method.c_str(), method.size(), attr.requestMethod, sizeof(attr.requestMethod));

	core::DynamicArray<const char*> headers;
	headers.reserve(_headers.size() + 1);
	if (!_userAgent.empty()) {
		headers.push_back("User-Agent");
		headers.push_back(_userAgent.c_str());
	}
	for (const auto &entry : _headers) {
		headers.push_back(entry->first.c_str());
		headers.push_back(entry->second.c_str());
	}
	headers.push_back(nullptr);
	attr.requestHeaders = headers.data();
	attr.timeoutMSecs = _timeoutSecond * 1000;

	if (!_body.empty()) {
		attr.requestData = _body.c_str();
		attr.requestDataSize = _body.size();
	}

	emscripten_fetch_t* fetch = emscripten_fetch(&attr, _url.c_str());
	if (fetch == nullptr) {
		Log::error("Http request for '%s' failed", _url.c_str());
		return false;
	}
	if (statusCode) {
		*statusCode = (int)fetch->status;
	}
	if (outheaders) {
		size_t headersLengthBytes = emscripten_fetch_get_response_headers_length(fetch) + 1;
		char *headerString = new char[headersLengthBytes];
		emscripten_fetch_get_response_headers(fetch, headerString, headersLengthBytes);
		char **responseHeaders = emscripten_fetch_unpack_response_headers(headerString);
		delete[] headerString;

		for (int numHeaders = 0; responseHeaders[numHeaders * 2]; ++numHeaders) {
			if (responseHeaders[(numHeaders * 2) + 1] == nullptr) {
				break;
			}

			core::String key = (responseHeaders[numHeaders * 2]) ? responseHeaders[numHeaders * 2] : "";
			core::String value =
				(responseHeaders[(numHeaders * 2) + 1]) ? responseHeaders[(numHeaders * 2) + 1] : "";
			outheaders->put(key, value);
		}

		emscripten_fetch_free_unpacked_response_headers(responseHeaders);
	}
	Log::debug("Got status code %i for %s", (int)fetch->status, _url.c_str());
	if (stream.write(fetch->data, fetch->numBytes) == -1) {
		Log::error("Failed to write response with %i bytes for url %s", (int)fetch->numBytes, _url.c_str());
		emscripten_fetch_close(fetch);
		return false;
	}
	emscripten_fetch_close(fetch);
	return true;
#elif USE_CURL
	CURL *curl = curl_easy_init();
	if (curl == nullptr) {
		return false;
	}

	curl_slist *headers = nullptr;
	for (const auto &entry : _headers) {
		const core::String &headerLine = core::string::format("%s: %s", entry->first.c_str(), entry->second.c_str());
		headers = curl_slist_append(headers, headerLine.c_str());
	}
	if (headers) {
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	}
	if (!_body.empty()) {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, _body.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, _type == RequestType::GET ? "GET" : "POST");
	curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
	if (outheaders) {
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeaderData);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, outheaders);
	}
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, _connectTimeoutSecond);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, _timeoutSecond);
	// curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3);
	if (!_userAgent.empty()) {
		curl_easy_setopt(curl, CURLOPT_USERAGENT, _userAgent.c_str());
	}
	const CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		char *url = nullptr;
		curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
		Log::error("Http request for '%s' failed with error %s", url, curl_easy_strerror(res));
	}
	long statusCodeCurl = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCodeCurl);
	Log::debug("Got status code %i for %s", (int)statusCodeCurl, _url.c_str());
	if (statusCode) {
		*statusCode = (int)statusCodeCurl;
	}
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	return res == CURLE_OK;
#endif
	return false;
}

} // namespace http
