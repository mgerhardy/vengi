/**
 * @file
 */

#include "core/Log.h"
#include "core/Var.h"
#include "core/StringUtil.h"
#include "engine-config.h"
#include "core/Common.h"
#include "core/Assert.h"
#include <string.h>
#include <stdio.h>

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifdef SDL_PLATFORM_LINUX
#define ANSI_COLOR_RESET "\033[0m"
#define ANSI_COLOR_RED "\033[31m"
#define ANSI_COLOR_GREEN "\033[32m"
#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_COLOR_BLUE "\033[34m"
#define ANSI_COLOR_CYAN "\033[36m"
#else
#define ANSI_COLOR_RESET ""
#define ANSI_COLOR_RED ""
#define ANSI_COLOR_GREEN ""
#define ANSI_COLOR_YELLOW ""
#define ANSI_COLOR_BLUE ""
#define ANSI_COLOR_CYAN ""
#endif

namespace priv {

static bool _syslog = false;
static FILE* _logfile = nullptr;
static constexpr int bufSize = 4096;
static Log::Level _logLevel = Log::Level::Info;

#ifdef HAVE_SYSLOG_H
static SDL_LogOutputFunction _syslogLogCallback = nullptr;
static void *_syslogLogCallbackUserData = nullptr;


static void sysLogOutputFunction(void *userdata, int category, SDL_LogPriority priority, const char *message) {
	int syslogLevel = LOG_DEBUG;
	if (priority == SDL_LOG_PRIORITY_CRITICAL) {
		syslogLevel = LOG_CRIT;
	} else if (priority == SDL_LOG_PRIORITY_ERROR) {
		syslogLevel = LOG_ERR;
	} else if (priority == SDL_LOG_PRIORITY_WARN) {
		syslogLevel = LOG_WARNING;
	} else if (priority == SDL_LOG_PRIORITY_INFO) {
		syslogLevel = LOG_INFO;
	}
	syslog(syslogLevel, "%s", message);
	if (_syslogLogCallback != sysLogOutputFunction) {
		_syslogLogCallback(_syslogLogCallbackUserData, category, priority, message);
	}
}
#endif
}

Log::Level Log::toLogLevel(const char* level) {
	const core::String string(level);
	if (core::string::iequals(string, "trace")) {
		return Level::Trace;
	}
	if (core::string::iequals(string, "debug")) {
		return Level::Debug;
	}
	if (core::string::iequals(string, "info")) {
		return Level::Info;
	}
	if (core::string::iequals(string, "warn")) {
		return Level::Warn;
	}
	if (core::string::iequals(string, "error")) {
		return Level::Error;
	}
	return Level::None;
}

const char* Log::toLogLevel(Log::Level level) {
	if (level == Level::Trace) {
		return "trace";
	}
	if (level == Level::Debug) {
		return "debug";
	}
	if (level == Level::Info) {
		return "info";
	}
	if (level == Level::Warn) {
		return "warn";
	}
	if (level == Level::Error) {
		return "error";
	}
	return "none";
}

void Log::setLevel(Level level) {
	SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)level);
}

void Log::init(const char *logfile) {
	int rawLogLevel = core::Var::getSafe(cfg::CoreLogLevel)->intVal();
	if (rawLogLevel < 0 || rawLogLevel > (int)Log::Level::Error) {
		rawLogLevel = (int)Log::Level::Error;
	}
	priv::_logLevel = (Log::Level)rawLogLevel;
	setLevel(Log::Level::Trace);

	if (priv::_logfile == nullptr && logfile != nullptr) {
		priv::_logfile = fopen(logfile, "w");
	}

	const bool syslog = core::Var::getSafe(cfg::CoreSysLog)->boolVal();
	if (syslog) {
#ifdef HAVE_SYSLOG_H
		if (!priv::_syslog) {
			if (priv::_syslogLogCallback == nullptr) {
				SDL_GetLogOutputFunction(&priv::_syslogLogCallback, &priv::_syslogLogCallbackUserData);
			}
			core_assert(priv::_syslogLogCallback != priv::sysLogOutputFunction);
			openlog(nullptr, LOG_PID, LOG_USER);
			SDL_SetLogOutputFunction(priv::sysLogOutputFunction, nullptr);
			priv::_syslog = true;
		}
#else
		Log::warn("Syslog support is not compiled into the binary");
		priv::_syslog = false;
#endif
	} else {
#ifdef HAVE_SYSLOG_H
		if (priv::_syslog) {
			SDL_SetLogOutputFunction(priv::_syslogLogCallback, priv::_syslogLogCallbackUserData);
			priv::_syslogLogCallback = nullptr;
			priv::_syslogLogCallbackUserData = nullptr;
			closelog();
		}
#endif
		priv::_syslog = false;
	}
}

void Log::shutdown() {
	// this is one of the last methods that is executed - so don't rely on anything
	// still being available here - it won't
#ifdef HAVE_SYSLOG_H
	if (priv::_syslog) {
		SDL_SetLogOutputFunction(priv::_syslogLogCallback, priv::_syslogLogCallbackUserData);
		closelog();
		priv::_syslogLogCallback = nullptr;
		priv::_syslogLogCallbackUserData = nullptr;
	}
#endif
	if (priv::_logfile) {
		fflush(priv::_logfile);
		fclose(priv::_logfile);
		priv::_logfile = nullptr;
	}
	priv::_logLevel = Log::Level::Info;
	priv::_syslog = false;
}

static void traceVA(const char *msg, va_list args) {
	char buf[priv::bufSize];
	SDL_vsnprintf(buf, sizeof(buf), msg, args);
	buf[sizeof(buf) - 1] = '\0';
	if (priv::_logfile) {
		fprintf(priv::_logfile, "[TRACE] %s\n", buf);
	}
	if (priv::_syslog) {
		SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "%s\n", buf);
	} else {
		SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n", buf);
	}
}

static void debugVA(const char *msg, va_list args) {
	char buf[priv::bufSize];
	SDL_vsnprintf(buf, sizeof(buf), msg, args);
	buf[sizeof(buf) - 1] = '\0';
	if (priv::_logfile) {
		fprintf(priv::_logfile, "[DEBUG] %s\n", buf);
	}
	if (priv::_syslog) {
		SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s\n", buf);
	} else {
		SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET "\n", buf);
	}
}

static void infoVA(const char *msg, va_list args) {
	char buf[priv::bufSize];
	SDL_vsnprintf(buf, sizeof(buf), msg, args);
	buf[sizeof(buf) - 1] = '\0';
	if (priv::_logfile) {
		fprintf(priv::_logfile, "[INFO] %s\n", buf);
	}
	if (priv::_syslog) {
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s\n", buf);
	} else {
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n", buf);
	}
}

static void warnVA(const char *msg, va_list args) {
	char buf[priv::bufSize];
	SDL_vsnprintf(buf, sizeof(buf), msg, args);
	buf[sizeof(buf) - 1] = '\0';
	if (priv::_logfile) {
		fprintf(priv::_logfile, "[WARN] %s\n", buf);
	}
	if (priv::_syslog) {
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s\n", buf);
	} else {
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "\n", buf);
	}
}

static void errorVA(const char *msg, va_list args) {
	char buf[priv::bufSize];
	SDL_vsnprintf(buf, sizeof(buf), msg, args);
	buf[sizeof(buf) - 1] = '\0';
	if (priv::_logfile) {
		fprintf(priv::_logfile, "[ERROR] %s\n", buf);
	}
	if (priv::_syslog) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", buf);
	} else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, ANSI_COLOR_RED "%s" ANSI_COLOR_RESET "\n", buf);
	}
}

static void printfVA(const char *msg, va_list args) {
	char buf[priv::bufSize];
	SDL_vsnprintf(buf, sizeof(buf), msg, args);
	buf[sizeof(buf) - 1] = '\0';
	if (priv::_logfile) {
		fprintf(priv::_logfile, "%s", buf);
	}
	printf("%s", buf);
}

void Log::trace(const char* msg, ...) {
	if (priv::_logLevel > Log::Level::Trace) {
		return;
	}
	va_list args;
	va_start(args, msg);
	traceVA(msg, args);
	va_end(args);
}

void Log::debug(const char* msg, ...) {
	if (priv::_logLevel > Log::Level::Debug) {
		return;
	}
	va_list args;
	va_start(args, msg);
	debugVA(msg, args);
	va_end(args);
}

void Log::info(const char* msg, ...) {
	if (priv::_logLevel > Log::Level::Info) {
		return;
	}
	va_list args;
	va_start(args, msg);
	infoVA(msg, args);
	va_end(args);
}

void Log::warn(const char* msg, ...) {
	if (priv::_logLevel > Log::Level::Warn) {
		return;
	}
	va_list args;
	va_start(args, msg);
	warnVA(msg, args);
	va_end(args);
}

void Log::error(const char* msg, ...) {
	if (priv::_logLevel > Log::Level::Error) {
		return;
	}
	va_list args;
	va_start(args, msg);
	errorVA(msg, args);
	va_end(args);
}

void Log::printf(const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	printfVA(msg, args);
	va_end(args);
}

void c_logtrace(const char* msg, ...) {
	if (priv::_logLevel > Log::Level::Trace) {
		return;
	}
	va_list args;
	va_start(args, msg);
	traceVA(msg, args);
	va_end(args);
}

void c_logdebug(const char* msg, ...) {
	if (priv::_logLevel > Log::Level::Debug) {
		return;
	}
	va_list args;
	va_start(args, msg);
	debugVA(msg, args);
	va_end(args);
}

void c_loginfo(const char* msg, ...) {
	if (priv::_logLevel > Log::Level::Info) {
		return;
	}
	va_list args;
	va_start(args, msg);
	infoVA(msg, args);
	va_end(args);
}

void c_logwarn(const char* msg, ...) {
	if (priv::_logLevel > Log::Level::Warn) {
		return;
	}
	va_list args;
	va_start(args, msg);
	warnVA(msg, args);
	va_end(args);
}

void c_logerror(CORE_FORMAT_STRING const char* msg, ...) {
	if (priv::_logLevel > Log::Level::Error) {
		return;
	}
	va_list args;
	va_start(args, msg);
	errorVA(msg, args);
	va_end(args);
}

extern "C" void c_logwrite(const char* msg, size_t length) {
	char buf[priv::bufSize];
	SDL_memcpy(buf, msg, core_max(priv::bufSize - 1, length));
	buf[sizeof(buf) - 1] = '\0';
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", buf);
}
