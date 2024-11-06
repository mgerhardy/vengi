/**
 * @file
 */

#include "Assert.h"
#include "Log.h"
#include "core/String.h"
#include "core/StringUtil.h"
#include <SDL3/SDL_assert.h>

#ifdef HAVE_BACKWARD
#include <backward.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static core::String g_crashLogPath = "crash.log";

const char *core_crashlog_path() {
	return g_crashLogPath.c_str();
}

static_assert(sizeof(SDL_AssertData) == sizeof(AssertData));

static SDL_AssertState coreAssertionHandler(const SDL_AssertData *data, void *userdata) {
	if (data->trigger_count <= 1 && data->always_ignore == 0) {
		core_stacktrace();
	}
	const SDL_AssertState state = SDL_GetDefaultAssertionHandler()(data, userdata);
	if (state == SDL_ASSERTION_RETRY) {
		return state;
	}
	if (state == SDL_ASSERTION_ABORT) {
		core_write_stacktrace();
		return state;
	}
	return state;
}

void core_assert_init(const char *crashLogDir) {
	if (crashLogDir != nullptr) {
		g_crashLogPath = core::string::path(crashLogDir, "crash.log");
	}
	if (SDL_GetAssertionHandler(nullptr) != coreAssertionHandler) {
		SDL_SetAssertionHandler(coreAssertionHandler, nullptr);
	}
}

bool core_report_assert(AssertData &data, const char *file, int line, const char *function) {
	const SDL_AssertState state = SDL_ReportAssertion((SDL_AssertData *)&data, function, file, line);
	if (state == SDL_ASSERTION_RETRY) {
		return true;
	}
	if (state == SDL_ASSERTION_BREAK) {
		SDL_TriggerBreakpoint();
	}
	return false;
}

bool core_assert_impl_message(AssertData &data, char *buf, int bufSize, const char *function, const char *file,
							  int line, const char *format, ...) {
	va_list args;
	va_start(args, format);
	SDL_vsnprintf(buf, bufSize - 1, format, args);
	va_end(args);
	data.condition = buf; /* also let it work for following calls */
	const SDL_AssertState state = SDL_ReportAssertion((SDL_AssertData *)&data, function, file, line);
	if (state == SDL_ASSERTION_BREAK) {
		SDL_TriggerBreakpoint();
	}

	return false;
}

void core_get_stacktrace(char *buf, size_t size) {
#ifdef HAVE_BACKWARD
	std::ostringstream os;
	backward::StackTrace st;
	st.load_here(32);
	backward::Printer printer;
	printer.print(st, os);
	const std::string &str = os.str();
	if (str.size() + 1 < size) {
		size = str.size() + 1;
	}
	memcpy(buf, str.c_str(), size);
	buf[size - 1] = '\0';
#endif
}

void core_write_stacktrace(const char *file) {
#if defined(HAVE_BACKWARD)
	core::String p = file == nullptr ? g_crashLogPath.c_str() : file;
	if (p.empty()) {
		p = "crash.log";
	}
	printf("write crash to file: %s\n", p.c_str());
	std::ofstream os(p.c_str());
	backward::StackTrace st;
	st.load_here(32);
	backward::Printer printer;
	printer.print(st, os);
#endif
}

void core_stacktrace() {
#ifdef __EMSCRIPTEN__
	EM_ASM({ stackTrace(); });
#elif defined(HAVE_BACKWARD)
	std::ostringstream os;
	backward::StackTrace st;
	st.load_here(32);
	backward::Printer printer;
	printer.print(st, os);
	std::string str = os.str();
	const char *c = str.c_str();
	while (char *l = SDL_strchr(c, '\n')) {
		*l = '\0';
		Log::error("%s", c);
		c = l + 1;
	}
#endif
}
