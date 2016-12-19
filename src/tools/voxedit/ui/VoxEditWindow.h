/**
 * @file
 */

#pragma once

#include "ui/Window.h"
#include "core/Common.h"
#include "core/String.h"
#include "editorscene/Axis.h"
#include "voxel/TreeContext.h"
#include "editorscene/Action.h"
#include "editorscene/SelectType.h"

class EditorScene;
class VoxEdit;
class PaletteWidget;

namespace voxedit {

/**
 * @brief Voxel editing tools panel
 */
class VoxEditWindow: public ui::Window {
	friend class ::VoxEdit;
private:
	EditorScene* _scene;
	EditorScene* _sceneTop = nullptr;
	EditorScene* _sceneLeft = nullptr;
	EditorScene* _sceneFront = nullptr;
	VoxEdit* _voxedit;
	PaletteWidget* _paletteWidget;
	tb::TBWidget* _exportButton = nullptr;
	tb::TBWidget* _saveButton = nullptr;
	tb::TBWidget* _undoButton = nullptr;
	tb::TBWidget* _redoButton = nullptr;

	tb::TBEditField* _cursorX = nullptr;
	tb::TBEditField* _cursorY = nullptr;
	tb::TBEditField* _cursorZ = nullptr;

	tb::TBCheckBox* _lockedX = nullptr;
	tb::TBCheckBox* _lockedY = nullptr;
	tb::TBCheckBox* _lockedZ = nullptr;

	std::string _voxelizeFile;
	std::string _loadFile;

	tb::TBSelectItemSourceList<tb::TBGenericStringItem> _treeItems;
	tb::TBSelectItemSourceList<tb::TBGenericStringItem> _fileItems;
	tb::TBSelectItemSourceList<tb::TBGenericStringItem> _structureItems;
	tb::TBSelectItemSourceList<tb::TBGenericStringItem> _plantItems;
	tb::TBSelectItemSourceList<tb::TBGenericStringItem> _buildingItems;

	tb::TBCheckBox *_showGrid = nullptr;
	tb::TBCheckBox *_showAABB = nullptr;
	tb::TBCheckBox *_showAxis = nullptr;
	tb::TBCheckBox *_freeLook = nullptr;

	std::string _exportFilter;
	bool _fourViewAvailable = false;
	bool _lockedDirty = false;

	glm::ivec3 _lastCursorPos;

	tb::TBGenericStringItem* addMenuItem(tb::TBSelectItemSourceList<tb::TBGenericStringItem>& items, const char *text, const char *id = nullptr);
	bool handleEvent(const tb::TBWidgetEvent &ev);

	enum class ModifierMode {
		None,
		Rotate,
		Scale,
		Move,
		Lock
	};
	ModifierMode _mode = ModifierMode::None;
	voxedit::Axis _axis = voxedit::Axis::None;
	static constexpr int MODENUMBERBUFSIZE = 64;
	char _modeNumberBuf[MODENUMBERBUFSIZE];
	long _lastModePress = -1l;
	void executeMode();
	void createTree(voxel::TreeType type);

	void setQuadViewport(bool active);
	void setSelectionType(SelectType type);
	void setAction(Action action);

	bool handleClickEvent(const tb::TBWidgetEvent &ev);
	bool handleChangeEvent(const tb::TBWidgetEvent &ev);
	void resetcamera();
	void quit();

	// commands
	void copy();
	void paste();
	void cut();
	void undo();
	void redo();
	void rotatex();
	void rotatey();
	void rotatez();
	void rotate(int x, int y, int z);
	void scalex();
	void scaley();
	void scalez();
	void scale(float x, float y, float z);
	void movex();
	void movey();
	void movez();
	void move(int x, int y, int z);
	void crop();
	void fill(int x, int y, int z);
	void extend(int size = 1);
	void setCursorPosition(int x, int y, int z);
	void toggleviewport();
	void togglefreelook();
	void movemode();
	void scalemode();
	void rotatemode();
	void togglelockaxis();
	void unselectall();
	bool voxelize(std::string_view file);
	bool save(std::string_view file);
	bool load(std::string_view file);
	bool exportFile(std::string_view file);
	bool createNew(bool force);
	void select(const glm::ivec3& pos);

public:
	VoxEditWindow(VoxEdit* tool);
	~VoxEditWindow();
	bool init();

	bool OnEvent(const tb::TBWidgetEvent &ev) override;
	void OnProcess() override;
	void OnDie() override;
};

inline void VoxEditWindow::rotatex() {
	rotate(90, 0, 0);
}

inline void VoxEditWindow::rotatey() {
	rotate(0, 90, 0);
}

inline void VoxEditWindow::rotatez() {
	rotate(0, 0, 90);
}

inline void VoxEditWindow::movex() {
	move(1, 0, 0);
}

inline void VoxEditWindow::movey() {
	move(0, 1, 0);
}

inline void VoxEditWindow::movez() {
	move(0, 0, 1);
}

inline void VoxEditWindow::scalex() {
	scale(2.0f, 0.0f, 0.0f);
}

inline void VoxEditWindow::scaley() {
	scale(0.0f, 2.0f, 0.0f);
}

inline void VoxEditWindow::scalez() {
	scale(0.0f, 0.0f, 2.0f);
}


}
