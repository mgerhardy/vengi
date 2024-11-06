/**
 * @file
 */

#include "WindowedApp.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "command/Command.h"
#include "core/Common.h"
#include "core/Log.h"
#include "core/Process.h"
#include "core/Singleton.h"
#include "core/StringUtil.h"
#include "app/I18N.h"
#include "io/BufferedReadWriteStream.h"
#include "video/Trace.h"
#include "core/TimeProvider.h"
#include "core/Var.h"
#include "io/FormatDescription.h"
#include "io/Filesystem.h"
#include "util/CustomButtonNames.h"
#include "util/KeybindingHandler.h"
#include "video/EventHandler.h"
#include <glm/common.hpp>
#include <SDL3/SDL.h>

#ifdef SDL_PLATFORM_WINDOWS
#include <windows.h>
#elif SDL_PLATFORM_MACOS
extern "C" bool isOSXDarkMode();
#endif

namespace video {

namespace {
inline void checkSDLError(const char *file, unsigned int line, const char *function) {
	const char *error = SDL_GetError();
	if (*error != '\0') {
		Log::error("%s (%s:%i => %s)", error, file, line, function);
		SDL_ClearError();
	} else {
		Log::error("unknown error (%s:%i => %s)", file, line, function);
	}
}
#define sdlCheckError() checkSDLError(__FILE__, __LINE__, SDL_FUNCTION)
}

WindowedApp::WindowedApp(const io::FilesystemPtr& filesystem, const core::TimeProviderPtr& timeProvider, size_t threadPoolSize) :
		Super(filesystem, timeProvider, threadPoolSize), _frameBufferDimension(-1), _mouseRelativePos(-1) {
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__) || (defined(SDL_PLATFORM_APPLE) && TARGET_OS_IOS)
	_singleWindowMode = true;
#else
	_singleWindowMode = false;
#endif
	core::registerBindingContext("ui", core::BindingContext::UI);
	core::registerBindingContext("all", core::BindingContext::All);
}

WindowedApp::~WindowedApp() {
	core::resetBindingContexts();
}

void WindowedApp::onAfterRunning() {
	core_trace_scoped(WindowedAppAfterRunning);
	video::endFrame(_window);
}

bool WindowedApp::handleSDLEvent(SDL_Event& event) {
	switch (event.type) {
	case SDL_EVENT_QUIT:
		// continue to handle any other following event
		return true;
	case SDL_EVENT_WINDOW_RESIZED:
	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
		SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
		// we must be the first to handle this - but others should get their chance, too
		if (window == _window) {
			const int w = event.window.data1;
			const int h = event.window.data2;
			int frameBufferWidth, frameBufferHeight;
			SDL_GetWindowSizeInPixels(_window, &frameBufferWidth, &frameBufferHeight);
			_aspect = (float)frameBufferWidth / (float)frameBufferHeight;
			_frameBufferDimension = glm::ivec2(frameBufferWidth, frameBufferHeight);
			_windowDimension = glm::ivec2(w, h);
			const float scaleFactor = (float)_frameBufferDimension.x / (float)_windowDimension.x;
			video::resize(w, h, scaleFactor);
			video::viewport(0, 0, _frameBufferDimension.x, _frameBufferDimension.y);
		}
		break;
	}
	case SDL_EVENT_WINDOW_MOVED:
		SDL_RaiseWindow(SDL_GetWindowFromID(event.window.windowID));
		// fallthrough
	default: {
		core_trace_scoped(WindowedAppEventHandler);
		const bool running = core::Singleton<video::EventHandler>::getInstance().handleEvent(event);
		if (!running) {
			return true;
		}
		break;
	}
	}
	return false;
}

app::AppState WindowedApp::onRunning() {
	video_trace_scoped(Frame);
	core_trace_scoped(WindowedAppOnRunning);
	if (isPressed(util::button::CUSTOM_SDLK_MOUSE_WHEEL_UP)) {
		handleKeyRelease(util::button::CUSTOM_SDLK_MOUSE_WHEEL_UP, 0);
	}
	if (isPressed(util::button::CUSTOM_SDLK_MOUSE_WHEEL_DOWN)) {
		handleKeyRelease(util::button::CUSTOM_SDLK_MOUSE_WHEEL_DOWN, 0);
	}
	if (isPressed(util::button::CUSTOM_SDLK_MOUSE_WHEEL_LEFT)) {
		handleKeyRelease(util::button::CUSTOM_SDLK_MOUSE_WHEEL_LEFT, 0);
	}
	if (isPressed(util::button::CUSTOM_SDLK_MOUSE_WHEEL_RIGHT)) {
		handleKeyRelease(util::button::CUSTOM_SDLK_MOUSE_WHEEL_RIGHT, 0);
	}

	// ignore the state here
	Super::onRunning();

	SDL_GetRelativeMouseState(&_mouseRelativePos.x, &_mouseRelativePos.y);
	SDL_Event event;
	bool quit = false;
	// we are checking the non headless flag here because we assume that a headless windowed
	// application is trying to e.g. render off-screen but without hidden timeouts
	if (_powerSaveMode && _showWindow) {
		bool windowIsHidden = SDL_GetWindowFlags(_window) & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED);
		while (windowIsHidden) {
			if (SDL_WaitEvent(&event) == 1) {
				quit = handleSDLEvent(event);
				windowIsHidden = SDL_GetWindowFlags(_window) & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED);
			}
		}
	}
	while (SDL_PollEvent(&event)) {
		quit |= handleSDLEvent(event);
	}

	if (quit) {
		Log::debug("Quitting the application");
		requestQuit();
		return app::AppState::Running;
	}

	core_trace_scoped(WindowedAppStartFrame);
	video::startFrame(_window, _rendererContext);
	core::Singleton<ShaderManager>::getInstance().update();

	const uint64_t end = core::TimeProvider::highResTime();
	const double frameDelta = (double)(end - _timeProvider->highResTickTime()) / (double)core::TimeProvider::highResTimeResolution() * 1000.0;
	_fps = 1.0 / frameDelta;

	video_trace_frame_end();

	return app::AppState::Running;
}

void WindowedApp::onWindowClose(void *windowHandle) {
	// if the main window is going to get closed, quit the application
	if (_window == windowHandle) {
		requestQuit();
	}
}

// https://stackoverflow.com/questions/25207077/how-to-detect-if-os-x-is-in-dark-mode
// https://wiki.archlinux.org/title/Dark_mode_switching#gsettings
bool WindowedApp::isDarkMode() const {
#ifdef SDL_PLATFORM_MACOS
	return isOSXDarkMode();
#elif SDL_PLATFORM_LINUX
	core::DynamicArray<core::String> arguments;
	arguments.push_back("get");
	arguments.push_back("org.gnome.desktop.interface");
	arguments.push_back("gtk-theme");
	io::BufferedReadWriteStream stream(4096);
	const int exitCode = core::Process::exec("/usr/bin/gsettings", arguments, nullptr, &stream);
	if (exitCode == 0) {
		stream.seek(0);
		core::String output;
		stream.readString(stream.size(), output);
		return core::string::icontains(output, "dark");
	}
	return true;
#elif SDL_PLATFORM_WINDOWS
	HKEY hkey;
	if (RegOpenKey(HKEY_CURRENT_USER, R"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)", &hkey) == ERROR_SUCCESS) {
		DWORD value = 0;
		DWORD size = sizeof(DWORD);
		auto error = RegQueryValueEx(hkey, "AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &size);
		if (error == ERROR_SUCCESS) {
			return value != 0;
		}
	}
	return true;
#else
	return true;
#endif
}

bool WindowedApp::onKeyRelease(void *windowHandle, int32_t key, int16_t modifier) {
	return handleKeyRelease(key, modifier);
}

bool WindowedApp::handleKeyRelease(int32_t key, int16_t /*modifier*/) {
	// don't use modifier here - this is the modifier that was released. But
	// we need the currently pressed modifier mask
	return _keybindingHandler.execute(key, SDL_GetModState(), false, nowSeconds());
}

bool WindowedApp::handleKeyPress(int32_t key, int16_t modifier, uint16_t count) {
	return _keybindingHandler.execute(key, modifier, true, nowSeconds(), count);
}

bool WindowedApp::onMouseWheel(void *windowHandle, float x, float y, int32_t mouseId) {
	int32_t key;
	if (y < 0.0f)
		key = util::button::CUSTOM_SDLK_MOUSE_WHEEL_UP;
	else if (y > 0.0f)
		key = util::button::CUSTOM_SDLK_MOUSE_WHEEL_DOWN;
	else if (x < 0.0f)
		key = util::button::CUSTOM_SDLK_MOUSE_WHEEL_LEFT;
	else if (x > 0.0f)
		key = util::button::CUSTOM_SDLK_MOUSE_WHEEL_RIGHT;
	else
		return false;
	const int16_t mod = SDL_GetModState();
	if (handleKeyPress(key, mod)) {
		return true;
	}
	return false;
}

void WindowedApp::onMouseButtonPress(void *windowHandle, int32_t x, int32_t y, uint8_t button, uint8_t clicks, int32_t mouseId) {
	handleKeyPress(CUSTOM_SDL_KEYCODE(button), SDL_GetModState(), clicks);
}

void WindowedApp::onMouseButtonRelease(void *windowHandle, int32_t x, int32_t y, uint8_t button, int32_t mouseId) {
	handleKeyRelease(CUSTOM_SDL_KEYCODE(button), SDL_GetModState());
}

bool WindowedApp::onKeyPress(void *windowHandle, int32_t key, int16_t modifier) {
	return handleKeyPress(key, modifier);
}

core::String WindowedApp::getKeyBindingsString(const char *cmd) const {
	return _keybindingHandler.getKeyBindingsString(cmd);
}

SDL_Window *WindowedApp::createWindow(int width, int height, int displayIndex, uint32_t flags) {
	const core::String windowName = fullAppname();
	// TODO: SDL3: set position SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex)
	//       create the window with properties
	return SDL_CreateWindow(windowName.c_str(), width, height, flags);
}

app::AppState WindowedApp::onInit() {
	app::AppState state = Super::onInit();
	if (state != app::AppState::Running) {
		return state;
	}

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		sdlCheckError();
		return app::AppState::InitFailure;
	}

	if (!_keybindingHandler.init()) {
		Log::error("Failed to initialize the key binding handler");
		return app::AppState::InitFailure;
	}
	if (!_keybindingHandler.load(_keybindingsVersion)) {
		Log::debug("Failed to load the keybindings");
	}

	core::Singleton<video::EventHandler>::getInstance().registerObserver(this);

	Log::debug("CPU count: %d", SDL_GetNumLogicalCPUCores());
	Log::debug("CacheLine size: %d", SDL_GetCPUCacheLineSize());
	Log::debug("Altivec: %d", SDL_HasAltiVec());
	Log::debug("MMX: %d", SDL_HasMMX());
	Log::debug("SSE: %d", SDL_HasSSE());
	Log::debug("SSE2: %d", SDL_HasSSE2());
	Log::debug("SSE3: %d", SDL_HasSSE3());
	Log::debug("SSE4.1: %d", SDL_HasSSE41());
	Log::debug("SSE4.2: %d", SDL_HasSSE42());
	Log::debug("AVX: %d", SDL_HasAVX());
	Log::debug("AVX2: %d", SDL_HasAVX2());
	Log::debug("NEON: %d", SDL_HasNEON());
	Log::debug("RAM: %d MB", SDL_GetSystemRAM());

	int numDisplays = 0;
	SDL_DisplayID *displays = SDL_GetDisplays(&numDisplays);
	if (displays != nullptr) {
		SDL_free(displays);
	}
	const int displayIndex = glm::clamp(core::Var::getSafe(cfg::ClientWindowDisplay)->intVal(), 0, core_max(0, numDisplays - 1));
	Log::debug("Try to use display %i", displayIndex);

	video::setup();

	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#ifdef SDL_HINT_MOUSE_DOUBLE_CLICK_TIME
	SDL_SetHint(SDL_HINT_MOUSE_DOUBLE_CLICK_TIME, "500");
#endif
#ifdef SDL_HINT_MOUSE_DOUBLE_CLICK_RADIUS
	SDL_SetHint(SDL_HINT_MOUSE_DOUBLE_CLICK_RADIUS, "32");
#endif
	SDL_SetHint(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, "1");

	int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	if (!_showWindow) {
		flags |= SDL_WINDOW_HIDDEN;
	}
	SDL_Rect displayBounds;
	displayBounds.h = displayBounds.w = displayBounds.x = displayBounds.y = 0;
	if (_fullScreenApplication) {
		if (!SDL_GetDisplayUsableBounds((SDL_DisplayID)displayIndex, &displayBounds)) {
			Log::error("Failed to query usable display bounds: %s", SDL_GetError());
			displayBounds.h = displayBounds.w = displayBounds.x = displayBounds.y = 0;
		}
	}
	const core::VarPtr& highDPI = core::Var::getSafe(cfg::ClientWindowHighDPI);
	if (highDPI->boolVal()) {
		flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
		Log::debug("Enable high dpi support");
	} else {
		Log::debug("Disable high dpi support");
	}

	const int videoDrivers = SDL_GetNumVideoDrivers();
	for (int i = 0; i < videoDrivers; ++i) {
		Log::debug("available driver: %s", SDL_GetVideoDriver(i));
	}

	Log::debug("driver: %s", SDL_GetCurrentVideoDriver());
	Log::debug("found %i displays (use %i at %i:%i)", numDisplays, displayIndex, displayBounds.x, displayBounds.y);

	const int width = core_max(_windowWidth, displayBounds.w);
	const int height = core_max(_windowHeight, displayBounds.h);
	_window = createWindow(width, height, displayIndex, flags);
	if (!_window) {
		Log::warn("Failed to get multisampled window - try to disable it");
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		_window = createWindow(width, height, displayIndex, flags);
		if (!_window) {
			sdlCheckError();
			return app::AppState::InitFailure;
		}
	}

	if (_fullScreenApplication) {
		SDL_MaximizeWindow(_window);
	}

	if (displayIndex != (int)SDL_GetDisplayForWindow(_window)) {
		Log::error("Failed to create window at display %i", displayIndex);
	}

	_rendererContext = video::createContext(_window);
	if (_rendererContext == nullptr) {
		sdlCheckError();
		return app::AppState::InitFailure;
	}

	// some platforms may override or hardcode the resolution - so
	// we have to query it here to get the actual resolution
	int frameBufferWidth, frameBufferHeight;
	SDL_GetWindowSizeInPixels(_window, &frameBufferWidth, &frameBufferHeight);
	_aspect = (float)frameBufferWidth / (float)frameBufferHeight;
	_frameBufferDimension = glm::ivec2(frameBufferWidth, frameBufferHeight);

	int windowWidth, windowHeight;
	SDL_GetWindowSize(_window, &windowWidth, &windowHeight);
	_windowDimension = glm::ivec2(windowWidth, windowHeight);

	Log::debug("resolution (%i:%i) (pixel)", _frameBufferDimension.x, _frameBufferDimension.y);
	Log::debug("resolution (%i:%i) (screen)", _windowDimension.x, _windowDimension.y);

	const float scaleFactor = (float)_frameBufferDimension.x / (float)_windowDimension.x;
	video::init(_windowDimension.x, _windowDimension.y, scaleFactor);
	video::viewport(0, 0, _frameBufferDimension.x, _frameBufferDimension.y);

	video_trace_init();

	return state;
}

app::AppState WindowedApp::onConstruct() {
	app::AppState state = Super::onConstruct();
	core::Var::get(cfg::ClientMultiSampleBuffers, "0");
	core::Var::get(cfg::ClientMultiSampleSamples, "0");
	core::Var::get(cfg::ClientWindowHighDPI, "false", core::CV_READONLY);
	core::Var::get(cfg::ClientShadowMap, "true", core::CV_SHADER, _("Activate shadow map"), core::Var::boolValidator);
	core::Var::get(cfg::RenderCheckerBoard, "false", core::CV_SHADER, _("Activate checkerboard rendering"), core::Var::boolValidator);
	core::Var::get(cfg::ClientBloom, "true", _("Activate bloom post processing"), core::Var::boolValidator);
	core::Var::get(cfg::ClientDebugShadow, "false", core::CV_SHADER, _("Activate shadow debug rendering"), core::Var::boolValidator);
	core::Var::get(cfg::ClientShadowMapSize, "1024");
	core::Var::get(cfg::ClientDebugShadowMapCascade, "false", core::CV_SHADER, _("Activate cascade shadow map debug rendering"), core::Var::boolValidator);
	core::Var::get(cfg::ClientGamma, "1.0", core::CV_SHADER, _("Gamma correction"));
	core::Var::get(cfg::ClientWindowDisplay, 0);
	core::Var::get(cfg::ClientOpenGLVersion, "3.3", core::CV_READONLY);
	core::Var::get(cfg::ClientMouseRotationSpeed, "0.01");
	core::Var::get(cfg::RenderOutline, "false", core::CV_SHADER, _("Render voxel outline"), core::Var::boolValidator);
	core::Var::get(cfg::RenderNormals, "false", core::CV_SHADER, _("Render voxel normals"), core::Var::boolValidator);
	core::Var::get(cfg::ToneMapping, "0", core::CV_SHADER, _("Activate tone mapping"), core::Var::minMaxValidator<0, 3>);
	core::Var::get(cfg::ClientVSync, "true", _("Limit the framerate to the monitor refresh rate"), core::Var::boolValidator);
	core::Var::get(cfg::ClientDebugSeverity, "0", 0u, _("0 disables it, 1 only highest severity, 2 medium severity, 3 everything"));
	core::Var::get(cfg::ClientCameraZoomSpeed, "0.1");

	core::Var::get(cfg::ClientCameraMaxZoom, "1000.0", 0u, "", [](const core::String &val) {
		const float fv = core::string::toFloat(val);
		return fv > 1.0f && fv <= 1000.0f;
	});
	core::Var::get(cfg::ClientCameraMinZoom, "0.1", 0u, "", [](const core::String &val) {
		const float fv = core::string::toFloat(val);
		return fv > 0.0001f && fv < 1000.0f;
	});

	command::Command::registerCommand("minimize", [&] (const command::CmdArgs& args) {
		minimize();
	}).setHelp(_("Minimize the window"));

	video::construct();

	_keybindingHandler.construct();

	return state;
}

app::AppState WindowedApp::onCleanup() {
	core::Singleton<video::EventHandler>::getInstance().removeObserver(this);
	video::destroyContext(_rendererContext);
	if (_window != nullptr) {
		SDL_DestroyWindow(_window);
	}
	SDL_Quit();
	video_trace_shutdown();

	_keybindingHandler.shutdown(_keybindingsVersion);

	return Super::onCleanup();
}

void WindowedApp::resetKeybindings() {
	_keybindingHandler.reset(_keybindingsVersion);
}

void WindowedApp::showCursor(bool show) {
	if (show) {
		SDL_ShowCursor();
	} else {
		SDL_HideCursor();
	}
}

void WindowedApp::centerMousePosition() {
	SDL_WarpMouseInWindow(_window, frameBufferWidth() / 2, frameBufferHeight() / 2);
}

bool WindowedApp::isRelativeMouseMode() const {
	return SDL_GetWindowRelativeMouseMode(_window);
}

bool WindowedApp::toggleRelativeMouseMode() {
	const bool current = isRelativeMouseMode();
	return setRelativeMouseMode(current ? false : true);
}

bool WindowedApp::setRelativeMouseMode(bool mode) {
	if (mode && !_allowRelativeMouseMode) {
		mode = false;
	}
	SDL_SetWindowRelativeMouseMode(_window, mode);
	return mode;
}

void WindowedApp::fileDialog(const FileDialogSelectionCallback& callback, const FileDialogOptions& options, OpenFileMode mode, const io::FormatDescription* formats, const core::String &filename) {
	Log::warn("This is not implemented in the base windowed application");
}

void WindowedApp::minimize() {
	SDL_MinimizeWindow(_window);
}

WindowedApp* WindowedApp::getInstance() {
	return (WindowedApp*)Super::getInstance();
}

}
