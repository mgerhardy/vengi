/**
 * @file
 */

#include "StampBrush.h"
#include "app/App.h"
#include "app/I18N.h"
#include "command/Command.h"
#include "core/Log.h"
#include "io/File.h"
#include "io/FileStream.h"
#include "io/FilesystemArchive.h"
#include "io/FormatDescription.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/SceneGraphNode.h"
#include "voxedit-util/Clipboard.h"
#include "voxedit-util/SceneManager.h"
#include "voxedit-util/modifier/ModifierVolumeWrapper.h"
#include "voxel/RawVolume.h"
#include "voxel/Voxel.h"
#include "voxelformat/Format.h"
#include "voxelformat/VolumeFormat.h"
#include "voxelutil/VolumeCropper.h"
#include "voxelutil/VolumeResizer.h"
#include "voxelutil/VolumeRotator.h"
#include "voxelutil/VolumeVisitor.h"
#include "voxelutil/VoxelUtil.h"
#include <glm/vector_relational.hpp>

namespace voxedit {

void StampBrush::construct() {
	command::Command::registerCommand("togglestampbrushcenter", [this](const command::CmdArgs &args) {
		_center ^= true;
	}).setHelp(_("Toggle center at cursor"));

	command::Command::registerCommand("togglestampbrushcontinuous", [this](const command::CmdArgs &args) {
		_continuous ^= true;
	}).setHelp(_("Toggle continuously placing the stamp voxels"));

	command::Command::registerCommand("stampbrushrotate", [this](const command::CmdArgs &args) {
		if (args.size() < 1) {
			Log::info("Usage: rotate <x|y|z>");
			return;
		}
		const math::Axis axis = math::toAxis(args[0]);
		_volume = voxelutil::rotateAxis(_volume, axis);
		_volume->translate(-_volume->region().getLowerCorner());
		markDirty();
	}).setHelp(_("Rotate stamp volume around the given axis"));

	command::Command::registerCommand("stampbrushuseselection", [this](const command::CmdArgs &) {
		Modifier &modifier = _sceneMgr->modifier();
		const Selections &selections = modifier.selections();
		if (!selections.empty()) {
			if (const scenegraph::SceneGraphNode *node =
					_sceneMgr->sceneGraphModelNode(_sceneMgr->sceneGraph().activeNode())) {
				const voxel::RawVolume stampVolume(node->volume(), selections);
				setVolume(stampVolume, node->palette());
				// we unselect here as it's not obvious for the user that the stamp also only operates in the selection
				// this can sometimes lead to confusion if you e.g. created a stamp from a fully filled selected area
				modifier.unselect();
			}
		}
	}).setHelp(_("Use the current selection as new stamp"));

	command::Command::registerCommand("stampbrushusenode", [this](const command::CmdArgs &) {
		const int activeNode = _sceneMgr->sceneGraph().activeNode();
		if (const scenegraph::SceneGraphNode *node = _sceneMgr->sceneGraphModelNode(activeNode)) {
			if (glm::all(glm::lessThan(node->region().getDimensionsInVoxels(), glm::ivec3(64)))) {
				setVolume(*node->volume(), node->palette());
			} else {
				Log::warn("Node is too large to be used as stamp");
			}
		}
	}).setHelp(_("Use the current selected node volume as new stamp"));

	command::Command::registerCommand("stampbrushpaste", [this](const command::CmdArgs &) {
		const voxel::VoxelData &clipBoard = _sceneMgr->clipBoardData();
		if (clipBoard) {
			setVolume(*clipBoard.volume, *clipBoard.palette);
		}
	}).setHelp(_("Paste the current clipboard content as stamp"));
}

voxel::Region StampBrush::calcRegion(const BrushContext &context) const {
	glm::ivec3 mins = context.cursorPosition;
	const glm::ivec3 &dim = _volume->region().getDimensionsInVoxels();
	if (_center) {
		mins -= dim / 2;
	} else {
		switch (context.cursorFace) {
		case voxel::FaceNames::NegativeX:
			mins.x -= (dim.x - 1);
			break;
		case voxel::FaceNames::NegativeY:
			mins.y -= (dim.y - 1);
			break;
		case voxel::FaceNames::NegativeZ:
			mins.z -= (dim.z - 1);
			break;
		default:
			break;
		}
	}
	glm::ivec3 maxs = mins + _volume->region().getDimensionsInCells();
	return voxel::Region(mins, maxs);
}

bool StampBrush::execute(scenegraph::SceneGraph &sceneGraph, ModifierVolumeWrapper &wrapper,
						 const BrushContext &context) {
	if (!_volume) {
		return false;
	}

	// TODO: context.lockedAxis support
	const voxel::Region &region = calcRegion(context);
	const glm::ivec3 &offset = region.getLowerCorner();
	voxelutil::visitVolume(*_volume, [&](int x, int y, int z, const voxel::Voxel &v) {
		wrapper.setVoxel(offset.x + x, offset.y + y, offset.z + z, v);
	});

	return true;
}

void StampBrush::reset() {
	Super::reset();
	_center = true;
	_continuous = false;
	_volume = nullptr;
}

void StampBrush::setSize(const glm::ivec3 &size) {
	if (glm::any(glm::greaterThan(size, glm::ivec3(MaxSize)))) {
		return;
	}
	if (_volume) {
		_volume = voxelutil::resize(_volume, voxel::Region(glm::ivec3(0), size - 1));
		_volume->translate(-_volume->region().getLowerCorner());
		markDirty();
	}
}

void StampBrush::setVolume(const voxel::RawVolume &volume, const palette::Palette &palette) {
	_volume = voxelutil::cropVolume(&volume);
	if (_volume) {
		if (glm::any(glm::greaterThan(_volume->region().getDimensionsInVoxels(), glm::ivec3(MaxSize)))) {
			Log::warn("Stamp size exceeds the max allowed size of 32x32x32");
			_volume = voxelutil::resize(_volume, voxel::Region(0, MaxSize));
		}
		_volume->translate(-_volume->region().getLowerCorner());
	}
	_palette = palette;
	markDirty();
}

void StampBrush::update(const BrushContext &ctx, double nowSeconds) {
	Super::update(ctx, nowSeconds);
	if (ctx.cursorPosition != _lastCursorPosition) {
		_lastCursorPosition = ctx.cursorPosition;
		markDirty();
	}
}

void StampBrush::setVoxel(const voxel::Voxel &voxel, const palette::Palette &palette) {
	if (_volume) {
		voxelutil::visitVolume(*_volume,
							   [&](int x, int y, int z, const voxel::Voxel &v) { _volume->setVoxel(x, y, z, voxel); });
	} else {
		_volume = new voxel::RawVolume(voxel::Region(glm::ivec3(0), glm::ivec3(0)));
		_volume->setVoxel(0, 0, 0, voxel);
		_palette = palette;
	}
	markDirty();
}

void StampBrush::convertToPalette(const palette::Palette &palette) {
	const voxel::Region &dirtyRegion = voxelutil::remapToPalette(_volume, _palette, palette);
	if (dirtyRegion.isValid()) {
		_palette = palette;
		markDirty();
	}
}

bool StampBrush::load(const core::String &filename) {
	const io::ArchivePtr &archive = io::openFilesystemArchive(io::filesystem());
	if (!archive->exists(filename)) {
		Log::error("Failed to open model file %s", filename.c_str());
		return false;
	}
	scenegraph::SceneGraph newSceneGraph;
	voxelformat::LoadContext loadCtx;
	io::FileDescription fileDesc;
	fileDesc.set(filename);
	if (!voxelformat::loadFormat(fileDesc, archive, newSceneGraph, loadCtx)) {
		Log::error("Failed to load %s", filename.c_str());
		return false;
	}
	scenegraph::SceneGraphNode *node = newSceneGraph.firstModelNode();
	if (!node) {
		Log::error("No model node found in %s", filename.c_str());
		return false;
	}
	voxel::RawVolume *v = node->volume();
	if (!v) {
		Log::error("No volume found in %s", filename.c_str());
		return false;
	}
	setVolume(*v, node->palette());
	return true;
}

} // namespace voxedit
