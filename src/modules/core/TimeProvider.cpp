/**
 * @file
 */

#include "TimeProvider.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <inttypes.h>
#include <SDL3/SDL.h>

namespace core {

static constexpr uint64_t SecToMillis = (uint64_t)1000;

void TimeProvider::updateTickTime() {
	const double freq = (double)highResTimeResolution();
	const uint64_t res = highResTimeResolution() / SecToMillis;
	_highResTime = highResTime();
	_tickSeconds = _highResTime / freq;
	_tickMillis = _highResTime / (double)res;
}

void TimeProvider::setTickTime(uint64_t tickMillis) {
	const double freq = (double)highResTimeResolution();
	const uint64_t res = highResTimeResolution() / SecToMillis;
	_highResTime = tickMillis * res;
	_tickSeconds = _highResTime / freq;
	_tickMillis = _highResTime / (double)res;
}

uint64_t TimeProvider::highResTimeResolution() {
	return SDL_GetPerformanceFrequency();
}

uint64_t TimeProvider::tickNow() const {
	return _highResTime / (highResTimeResolution() / SecToMillis);
}

uint64_t TimeProvider::systemMillis() {
	return highResTime() / (highResTimeResolution() / SecToMillis);
}

uint64_t TimeProvider::highResTime() {
	return SDL_GetPerformanceCounter();
}

core::String TimeProvider::toString(unsigned long millis, const char *format) {
	time_t t(millis / 1000UL);
	tm tm = *gmtime(&t);
	std::stringstream ss;
	ss << std::put_time(&tm, format);
	const std::string& tmp = ss.str();
	return core::String(tmp.c_str());
}

}
