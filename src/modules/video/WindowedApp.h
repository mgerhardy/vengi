/**
 * @file
 */

#pragma once

#include "app/App.h"
#include "video/IEventObserver.h"
#include "util/KeybindingHandler.h"
#include "video/Types.h"
#include "video/FileDialogOptions.h"
#include <SDL3/SDL_main.h>
#include <glm/vec2.hpp>

struct SDL_Window;
union SDL_Event;

namespace io {
struct FormatDescription;
}

namespace video {

/**
 * @brief An application with the usual lifecycle, but with a window attached to it.
 * @ingroup Video
 *
 * This application also receives input events (and others) from @c video::IEventObserver
 */
class WindowedApp: public app::App, public video::IEventObserver {
private:
	using Super = app::App;
protected:
	SDL_Window* _window = nullptr;
	RendererContext _rendererContext = nullptr;
	glm::ivec2 _frameBufferDimension;
	glm::ivec2 _windowDimension;
	float _aspect = 1.0f;
	double _fps = 0.0;
	bool _allowRelativeMouseMode = true;
	/**
	 * Allows to create hidden windows
	 */
	bool _showWindow = true;
	/**
	 * Don't allow to create new windows from without the application
	 */
	bool _singleWindowMode = false;
	/**
	 * Will block the event queue if the window is minimized of hidden
	 */
	bool _powerSaveMode = true;

	bool _fullScreenApplication = true;
	int _windowWidth = 1024;
	int _windowHeight = 768;

	/**
	 * Bump this if commands have changed that would make old keybindings invalid
	 */
	int _keybindingsVersion = 0;

	util::KeyBindingHandler _keybindingHandler;
	/**
	 * @brief Delta of the mouse movement since the last frame
	 */
	glm::vec2 _mouseRelativePos;

	WindowedApp(const io::FilesystemPtr& filesystem, const core::TimeProviderPtr& timeProvider, size_t threadPoolSize = 1);

	virtual bool handleSDLEvent(SDL_Event& event);

	inline bool isPressed(int32_t key) const { return _keybindingHandler.isPressed(key); };
	bool handleKeyPress(int32_t key, int16_t modifier, uint16_t count = 1u);
	bool handleKeyRelease(int32_t key, int16_t modifier);

	virtual ~WindowedApp();

	void showCursor(bool show);

	virtual SDL_Window *createWindow(int width, int height, int displayIndex, uint32_t flags);

	void centerMousePosition();

	bool toggleRelativeMouseMode();

	/**
	 * @brief (Un-)Grabs the mouse
	 */
	bool setRelativeMouseMode(bool mode);

	void resetKeybindings();

public:
	bool isRelativeMouseMode() const;
	bool isSingleWindowMode() const;

	void *windowHandle();
	const util::KeyBindingHandler &keybindingHandler() const;

	bool isDarkMode() const;

	/**
	 * @brief This may differ from windowDimension() if we're rendering to a high-DPI
	 * drawable, i.e. the window was created with high-DPI support (Apple calls this
	 * "Retina").
	 */
	const glm::ivec2& frameBufferDimension() const;
	/**
	 * @brief This is the window size
	 *
	 * The window size in screen coordinates may differ from the size in pixels, if
	 * the window was created with high-dpi support (e.g. iOS or OS X). Use
	 * pixelDimension() to get the real client area size in pixels.
	 */
	const glm::ivec2& windowDimension() const;
	int frameBufferWidth() const;
	int frameBufferHeight() const;

	core::String getKeyBindingsString(const char *cmd) const;

	/**
	 * @brief Opens a file dialog
	 * @param[in] mode @c OpenFileMode
	 * @param[in] formats nullptr terminated list of formats that are used to filter the entries
	 */
	virtual void fileDialog(const FileDialogSelectionCallback& callback, const FileDialogOptions& options, OpenFileMode mode, const io::FormatDescription* formats = nullptr, const core::String &filename = "");

	/**
	 * @brief Wrapper method for @c fileDialog()
	 * @param[in] formats nullptr terminated list of formats that are used to filter the entries
	 */
	void saveDialog(const FileDialogSelectionCallback& callback, const FileDialogOptions& options, const io::FormatDescription* formats = nullptr, const core::String &filename = "");
	/**
	 * @brief Wrapper method for @c fileDialog()
	 * @param[in] formats nullptr terminated list of formats that are used to filter the entries
	 */
	void openDialog(const FileDialogSelectionCallback& callback, const FileDialogOptions& options, const io::FormatDescription* formats = nullptr);
	/**
	 * @brief Wrapper method for @c fileDialog()
	 */
	void directoryDialog(const FileDialogSelectionCallback& callback, const FileDialogOptions& options);

	virtual app::AppState onRunning() override;
	virtual void onAfterRunning() override;
	virtual bool onMouseWheel(void *windowHandle, float x, float y, int32_t mouseId) override;
	virtual void onMouseButtonPress(void *windowHandle, int32_t x, int32_t y, uint8_t button, uint8_t clicks, int32_t mouseId) override;
	virtual void onMouseButtonRelease(void *windowHandle, int32_t x, int32_t y, uint8_t button, int32_t mouseId) override;
	virtual bool onKeyRelease(void *windowHandle, int32_t key, int16_t modifier) override;
	virtual bool onKeyPress(void *windowHandle, int32_t key, int16_t modifier) override;
	virtual app::AppState onConstruct() override;
	virtual app::AppState onInit() override;
	virtual app::AppState onCleanup() override;
	void onWindowClose(void *windowHandle) override;

	/**
	 * Minimize the application window but continue running
	 */
	void minimize();

	static double fps() {
		return getInstance()->_fps;
	}
	static WindowedApp* getInstance();
};

inline const util::KeyBindingHandler &WindowedApp::keybindingHandler() const {
	return _keybindingHandler;
}

inline void* WindowedApp::windowHandle() {
	return _window;
}

inline bool WindowedApp::isSingleWindowMode() const {
	return _singleWindowMode;
}

inline const glm::ivec2& WindowedApp::frameBufferDimension() const {
	return _frameBufferDimension;
}

inline const glm::ivec2& WindowedApp::windowDimension() const {
	return _windowDimension;
}

inline int WindowedApp::frameBufferWidth() const {
	return _frameBufferDimension.x;
}

inline int WindowedApp::frameBufferHeight() const {
	return _frameBufferDimension.y;
}

inline void WindowedApp::saveDialog(const FileDialogSelectionCallback& callback, const FileDialogOptions& options, const io::FormatDescription* formats, const core::String &filename) {
	fileDialog(callback, options, OpenFileMode::Save, formats, filename);
}

inline void WindowedApp::openDialog(const FileDialogSelectionCallback& callback, const FileDialogOptions& options, const io::FormatDescription* formats) {
	fileDialog(callback, options, OpenFileMode::Open, formats);
}

inline void WindowedApp::directoryDialog(const FileDialogSelectionCallback& callback, const FileDialogOptions& options) {
	fileDialog(callback, options, OpenFileMode::Directory);
}

}
