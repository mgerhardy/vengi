/**
 * @file
 */

#pragma once

#include "IMetricSender.h"
#include "core/String.h"
#include "core/concurrent/Lock.h"
#include "core/Trace.h"

#include <stdint.h>
#include <SDL3/SDL_platform.h>

#ifdef SDL_PLATFORM_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2spi.h>
#else
#define SOCKET  int
#endif

struct sockaddr_in;

namespace metric {

class UDPMetricSender : public IMetricSender {
private:
	const core::String _host;
	mutable SOCKET _socket;
	const uint16_t _port;
	mutable struct sockaddr_in* _statsd;
	mutable core_trace_mutex(core::Lock, _connectionMutex, "UDPMetricSender");

	bool connect() const;
public:
	UDPMetricSender(const core::String& host, int port);
	bool send(const char* buffer) const override;

	/**
	 * Connects to the port and host given by the cvars @c metric_port and @c metric_host.
	 */
	bool init() override;
	void shutdown() override;
};

}
