/**
 * @file
 */

#pragma once
#include <SDL3/SDL_main.h>
#include "io/Filesystem.h"
#include "core/TimeProvider.h"

#define TEST_APP(testClassName) \
int main(int argc, char *argv[]) { \
	const io::FilesystemPtr& filesystem = core::make_shared<io::Filesystem>(); \
	const core::TimeProviderPtr& timeProvider = core::make_shared<core::TimeProvider>(); \
	testClassName app(filesystem, timeProvider); \
	return app.startMainLoop(argc, argv); \
}
