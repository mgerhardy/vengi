/**
 * @file
 */

#include "AbstractBenchmark.h"
#include "core/Log.h"
#include "core/Var.h"
#include "command/Command.h"
#include "io/Filesystem.h"

#include <SDL3/SDL.h>

namespace app {

SDL_AssertState Test_AssertionHandler(const SDL_AssertData* data, void* userdata) {
	return SDL_ASSERTION_BREAK;
}

void AbstractBenchmark::SetUp(benchmark::State& st) {
	SDL_SetAssertionHandler(Test_AssertionHandler, nullptr);
	const io::FilesystemPtr filesystem = core::make_shared<io::Filesystem>();
	const core::TimeProviderPtr timeProvider = core::make_shared<core::TimeProvider>();
	_benchmarkApp = new BenchmarkApp(filesystem, timeProvider, this);
}

void AbstractBenchmark::TearDown(benchmark::State& st) {
	// prevent cvars from begin saved and reloaded for the next fiture in the test
	core::Var::shutdown();
	delete _benchmarkApp;
	_benchmarkApp = nullptr;
}

AbstractBenchmark::BenchmarkApp::BenchmarkApp(const io::FilesystemPtr& filesystem, const core::TimeProviderPtr& timeProvider, AbstractBenchmark* benchmark) :
		Super(filesystem, timeProvider), _benchmark(benchmark) {
	init(ORGANISATION, "benchmark");
	_initialLogLevel = Log::Level::Warn;
	while (_curState < AppState::Running) {
		core_trace_scoped(AppMainLoop);
		onFrame();
	}
}

AppState AbstractBenchmark::BenchmarkApp::onCleanup() {
	_benchmark->onCleanupApp();
	return Super::onCleanup();
}

AppState AbstractBenchmark::BenchmarkApp::onInit() {
	AppState state = Super::onInit();
	if (state != app::AppState::Running) {
		return state;
	}

	if (!_benchmark->onInitApp()) {
		return AppState::InitFailure;
	}

	return state;
}

AbstractBenchmark::BenchmarkApp::~BenchmarkApp() {
	while (AppState::InvalidAppState != _curState) {
		core_trace_scoped(AppMainLoop);
		onFrame();
	}
}

}
