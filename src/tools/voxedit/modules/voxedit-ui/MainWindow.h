/**
 * @file
 */

#pragma once

#include "core/collection/DynamicArray.h"
#include "io/FormatDescription.h"
#include "ui/IMGUIApp.h"
#include "ui/IMGUIEx.h"
#include "voxedit-ui/AnimationPanel.h"
#include "voxedit-ui/AnimationTimeline.h"
#include "voxedit-ui/AssetPanel.h"
#include "voxedit-ui/MementoPanel.h"
#include "voxedit-ui/PositionsPanel.h"
#include "voxedit-ui/LSystemPanel.h"
#include "voxedit-ui/QuitDisallowReason.h"
#include "voxedit-ui/SceneGraphPanel.h"
#include "voxedit-ui/MenuBar.h"
#include "voxedit-ui/ScriptPanel.h"
#include "voxedit-ui/ToolsPanel.h"
#include "voxedit-ui/PalettePanel.h"
#include "voxedit-ui/BrushPanel.h"
#include "voxedit-ui/StatusBar.h"
#include "voxedit-ui/TreePanel.h"
#include "voxedit-util/ModelNodeSettings.h"

#define ENABLE_RENDER_PANEL 0

namespace voxedit {

class Viewport;
struct TemplateModel;

class MainWindow {
private:
	core::VarPtr _lastOpenedFile;
	core::VarPtr _lastOpenedFiles;
	core::VarPtr _simplifiedView;
	core::VarPtr _numViewports;
	core::VarPtr _tipOfTheDay;

	core::DynamicArray<Viewport*> _scenes;
	Viewport* _lastHoveredScene = nullptr;

	bool _popupUnsaved = false;
	bool _popupNewScene = false;
	bool _popupFailedToSave = false;
	bool _popupVolumeSplit = false;
	bool _popupTipOfTheDay = false;
	bool _popupWelcome = false;
	bool _popupUnsavedChangesQuit = false;
	bool _forceQuit = false;
	bool _lastSceneMode = false;
	bool _isNewVersionAvailable = false;
	uint32_t _currentTip = 0;

	ui::IMGUIApp* _app;

	LastOpenedFiles _lastOpenedFilesRingBuffer;
	ModelNodeSettings _modelNodeSettings;
	io::FileDescription _loadFile;
	LSystemPanel _lsystemPanel;
	BrushPanel _brushPanel;
	TreePanel _treePanel;
	SceneGraphPanel _sceneGraphPanel;
	AnimationPanel _animationPanel;
#if ENABLE_RENDER_PANEL
	RenderPanel _renderPanel;
#endif
	ToolsPanel _toolsPanel;
	AssetPanel _assetPanel;
	MementoPanel _mementoPanel;
	PositionsPanel _positionsPanel;
	PalettePanel _palettePanel;
	MenuBar _menuBar;
	StatusBar _statusBar;
	ScriptPanel _scriptPanel;
	AnimationTimeline _animationTimeline;
	video::TexturePool _texturePool;

	/**
	 * @brief Convert semicolon-separated string into the @c _lastOpenedFilesRingBuffer array
	 */
	void loadLastOpenedFiles(const core::String &string);
	void addLastOpenedFile(const core::String &file);
	bool isSceneMode() const;

	void shutdownScenes();
	bool initScenes();

	void leftWidget();
	void mainWidget();
	void rightWidget();

	void configureMainBottomWidgetDock(ImGuiID dockId);
	void configureMainTopWidgetDock(ImGuiID dockId);
	void configureRightTopWidgetDock(ImGuiID dockId);
	void configureRightBottomWidgetDock(ImGuiID dockId);
	void configureLeftTopWidgetDock(ImGuiID dockId);
	void configureLeftBottomWidgetDock(ImGuiID dockId);

	void dialog(const char *icon, const char *text);

	const char *getTip() const;

	/**
	 * called directly after loading a file. But the loading is async and might not yet have finished
	 * @sa onNewScene()
	 */
	void afterLoad(const core::String &file);
	void checkPossibleVolumeSplit();
	void newSceneTemplates();
	void popupNewScene();
	void popupWelcome();
	void popupTipOfTheDay();
	void popupFailedSave();
	void popupUnsavedChanges();
	void popupUnsavedDiscard();
	void popupSceneSettings();
	void popupAbout();
	void popupModelNodeSettings();
	void popupVolumeSplit();
	void registerPopups();
	void addTemplate(const TemplateModel &model);
public:
	MainWindow(ui::IMGUIApp *app);
	virtual ~MainWindow();
	bool init();
	void shutdown();

	// commands
	bool save(const core::String &file, const io::FormatDescription *desc);
	bool load(const core::String &file, const io::FormatDescription *desc);
	bool createNew(bool force);

	bool isSceneGraphDropTarget() const;
	bool isPaletteWidgetDropTarget() const;

	void toggleScene();
	void resetCamera();
	/**
	 * called directly after the scene manager has finished loading a new scene
	 * @sa afterLoad()
	 */
	void onNewScene();
	void update();
	QuitDisallowReason allowToQuit();
	Viewport* hoveredScene();
	/**
	 * @brief Checks if any of the viewports is in edit mode
	 */
	bool isAnyEditMode() const;

	bool saveScreenshot(const core::String& file, const core::String &viewportId = "");
};

}
