/**
 * @file
 */

#include "CommandlineApp.h"
#include "core/Var.h"
#include "core/Log.h"
#include <SDL3/SDL_platform.h>
#ifdef SDL_PLATFORM_WINDOWS
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x4
#endif
#endif

namespace app {

CommandlineApp::CommandlineApp(const io::FilesystemPtr& filesystem, const core::TimeProviderPtr& timeProvider, size_t threadPoolSize) :
		Super(filesystem, timeProvider, threadPoolSize) {
#ifdef SDL_PLATFORM_WINDOWS
	// https://learn.microsoft.com/en-us/windows/console/setconsolemode
	HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdoutHandle != INVALID_HANDLE_VALUE) {
		DWORD console_mode;
		if (GetConsoleMode(stdoutHandle, &console_mode)) {
			SetConsoleMode(stdoutHandle, ENABLE_VIRTUAL_TERMINAL_PROCESSING | console_mode);
		}
	}
#endif
}

CommandlineApp::~CommandlineApp() {
}

AppState CommandlineApp::onConstruct() {
	const app::AppState state = Super::onConstruct();

	registerArg("--trace").setDescription("Change log level to trace");
	registerArg("--debug").setDescription("Change log level to debug");
	registerArg("--info").setDescription("Change log level to info");
	registerArg("--warn").setDescription("Change log level to warn");
	registerArg("--error").setDescription("Change log level to error");

	bool changed = false;
	if (hasArg("--trace")) {
		core::Var::getSafe(cfg::CoreLogLevel)->setVal(SDL_LOG_PRIORITY_VERBOSE);
		changed = true;
	} else if (hasArg("--debug")) {
		core::Var::getSafe(cfg::CoreLogLevel)->setVal(SDL_LOG_PRIORITY_DEBUG);
		changed = true;
	} else if (hasArg("--info")) {
		core::Var::getSafe(cfg::CoreLogLevel)->setVal(SDL_LOG_PRIORITY_INFO);
		changed = true;
	} else if (hasArg("--warn")) {
		core::Var::getSafe(cfg::CoreLogLevel)->setVal(SDL_LOG_PRIORITY_WARN);
		changed = true;
	} else if (hasArg("--error")) {
		core::Var::getSafe(cfg::CoreLogLevel)->setVal(SDL_LOG_PRIORITY_ERROR);
		changed = true;
	}
	if (changed) {
		Log::init();
	}
	return state;
}

}
