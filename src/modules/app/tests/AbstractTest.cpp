/**
 * @file
 */

#include "AbstractTest.h"
#include "core/Log.h"
#include "core/Var.h"
#include "core/TimeProvider.h"
#include "io/Filesystem.h"
#include <SDL3/SDL.h>

extern char **_argv;
extern int _argc;

namespace app {

core::String AbstractTest::fileToString(const core::String& filename) const {
	const io::FilesystemPtr& fs = _testApp->filesystem();
	const io::FilePtr& file = fs->open(filename);
	if (!file->exists()) {
		Log::error("Failed to load file '%s'", filename.c_str());
		return "";
	}
	return file->load();
}

void AbstractTest::SetUp() {
	const io::FilesystemPtr filesystem = core::make_shared<io::Filesystem>();
	const core::TimeProviderPtr timeProvider = core::make_shared<core::TimeProvider>();
	_testApp = new TestApp(filesystem, timeProvider, this);
	_testApp->run();
	const bool isRunning = _testApp->_curState == AppState::Running;
	ASSERT_TRUE(isRunning) << "Failed to setup the test app properly";
}

void AbstractTest::TearDown() {
	// prevent cvars from begin saved and reloaded for the next fixture in the test
	core::Var::shutdown();
	delete _testApp;
	_testApp = nullptr;
}

AbstractTest::TestApp::TestApp(const io::FilesystemPtr& filesystem, const core::TimeProviderPtr& timeProvider, AbstractTest* test) :
		Super(filesystem, timeProvider), _test(test) {
	init(ORGANISATION, "test");
	_initialLogLevel = Log::Level::Warn;
	_argc = ::_argc;
	_argv = ::_argv;
}

void AbstractTest::TestApp::run() {
	while (_curState < AppState::Running) {
		core_trace_scoped(AppMainLoop);
		onFrame();
	}
}

AppState AbstractTest::TestApp::onRunning() {
	Super::onRunning();
	return AppState::Running;
}

AppState AbstractTest::TestApp::onCleanup() {
	_test->onCleanupApp();
	return Super::onCleanup();
}

AppState AbstractTest::TestApp::onInit() {
	AppState state = Super::onInit();
	if (state != app::AppState::Running) {
		return state;
	}

	if (!_test->onInitApp()) {
		return AppState::InitFailure;
	}
#ifdef SDL_HINT_SHUTDOWN_DBUS_ON_QUIT
	SDL_SetHint(SDL_HINT_SHUTDOWN_DBUS_ON_QUIT, "1");
#endif

	return state;
}

AbstractTest::TestApp::~TestApp() {
	requestQuit();
	while (AppState::InvalidAppState != _curState) {
		core_trace_scoped(AppMainLoop);
		onFrame();
	}
	SDL_Quit();
}

}
