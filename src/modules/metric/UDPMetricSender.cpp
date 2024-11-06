/**
 * @file
 */

#include "UDPMetricSender.h"
#include "core/Log.h"
#include "core/Assert.h"
#include <string.h>
#ifdef SDL_PLATFORM_WINDOWS
#define network_cleanup() WSACleanup()
#define network_return int
#else
#define network_return ssize_t
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#define closesocket close
#define INVALID_SOCKET  (-1)
#define network_cleanup()
#endif

namespace metric {

UDPMetricSender::UDPMetricSender(const core::String& host, int port) :
		_host(host), _socket(INVALID_SOCKET), _port(port), _statsd(nullptr) {
}

bool UDPMetricSender::init() {
	Log::debug("metric udp sender %s:%i", _host.c_str(), (int)_port);
#ifdef WIN32
	WSADATA wsaData;
	const int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaResult != NO_ERROR) {
		return false;
	}
#endif
	connect();
	return true;
}

void UDPMetricSender::shutdown() {
	closesocket(_socket);
	network_cleanup();
	delete _statsd;
	_statsd = nullptr;
}

bool UDPMetricSender::connect() const {
	core::ScopedLock lock(_connectionMutex);
	if (_socket != INVALID_SOCKET) {
		return true;
	}

	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_socket == INVALID_SOCKET) {
		return false;
	}

	_statsd = new struct sockaddr_in;
	_statsd->sin_family = AF_INET;
	_statsd->sin_port = htons(_port);

	struct addrinfo hints;
	SDL_memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	struct addrinfo* results = nullptr;
	const int ret = getaddrinfo(_host.c_str(), nullptr, &hints, &results);
	if (ret != 0) {
		closesocket(_socket);
		_socket = INVALID_SOCKET;
		return false;
	}

	const struct sockaddr_in* host_addr = (const struct sockaddr_in*) results->ai_addr;
	SDL_memcpy(&_statsd->sin_addr, &host_addr->sin_addr, sizeof(_statsd->sin_addr));
	freeaddrinfo(results);
	return true;
}

bool UDPMetricSender::send(const char* buffer) const {
	if (_socket == INVALID_SOCKET) {
		if (!connect()) {
			return false;
		}
	}
	core_assert(_statsd != nullptr);
	core_assert(_socket != INVALID_SOCKET);
	const size_t len = (size_t)SDL_strlen(buffer);
	const struct sockaddr *sock = (const struct sockaddr *)_statsd;
	const socklen_t socksize = (socklen_t)sizeof(*_statsd);
#ifdef WIN32
	const int ret = (int)sendto(_socket, buffer, len, 0, sock, socksize);
#else
	const void* buf = (const void*)buffer;
	const int ret = sendto(_socket, buf, len, 0, sock, socksize);
#endif
	if (ret == -1) {
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
	return ret != -1;
}

}
