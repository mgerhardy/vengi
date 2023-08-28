/**
 * @file
 */

#include "ModifierRenderer.h"
#include "core/Color.h"
#include "math/Axis.h"
#include "core/Log.h"
#include "video/Camera.h"
#include "video/ScopedPolygonMode.h"
#include "video/ScopedState.h"
#include "video/ShapeBuilder.h"
#include "video/Types.h"
#include "voxedit-util/SceneManager.h"
#include "../AxisUtil.h"
#include "voxedit-util/modifier/Selection.h"
#include "voxel/Palette.h"

namespace voxedit {

bool ModifierRenderer::init() {
	if (!_shapeRenderer.init()) {
		Log::error("Failed to initialize the shape renderer");
		return false;
	}

	_volumeRenderer.construct();
	if (!_volumeRenderer.init()) {
		Log::error("Failed to initialize the volume renderer");
		return false;
	}

	_shapeBuilder.clear();
	_shapeBuilder.setColor(core::Color::alpha(core::Color::SteelBlue, 0.8f));
	_shapeBuilder.sphere(8, 6, 0.5f);
	_referencePointMesh = _shapeRenderer.create(_shapeBuilder);

	return true;
}

void ModifierRenderer::shutdown() {
	_mirrorMeshIndex = -1;
	_selectionIndex = -1;
	_voxelCursorMesh = -1;
	_referencePointMesh = -1;
	_shapeRenderer.shutdown();
	_shapeBuilder.shutdown();
	_volumeRendererCtx.shutdown();
	_volumeRenderer.shutdown();
}

void ModifierRenderer::updateCursor(const voxel::Voxel& voxel, voxel::FaceNames face, bool flip) {
	_shapeBuilder.clear();
	video::ShapeBuilderCube flags = video::ShapeBuilderCube::All;
	switch (face) {
	case voxel::FaceNames::PositiveX:
		if (flip) {
			flags = video::ShapeBuilderCube::Left;
		} else {
			flags = video::ShapeBuilderCube::Right;
		}
		break;
	case voxel::FaceNames::PositiveY:
		if (flip) {
			flags = video::ShapeBuilderCube::Bottom;
		} else {
			flags = video::ShapeBuilderCube::Top;
		}
		break;
	case voxel::FaceNames::PositiveZ:
		if (flip) {
			flags = video::ShapeBuilderCube::Back;
		} else {
			flags = video::ShapeBuilderCube::Front;
		}
		break;
	case voxel::FaceNames::NegativeX:
		if (flip) {
			flags = video::ShapeBuilderCube::Right;
		} else {
			flags = video::ShapeBuilderCube::Left;
		}
		break;
	case voxel::FaceNames::NegativeY:
		if (flip) {
			flags = video::ShapeBuilderCube::Top;
		} else {
			flags = video::ShapeBuilderCube::Bottom;
		}
		break;
	case voxel::FaceNames::NegativeZ:
		if (flip) {
			flags = video::ShapeBuilderCube::Front;
		} else {
			flags = video::ShapeBuilderCube::Back;
		}
		break;
	case voxel::FaceNames::Max:
		return;
	}
	_shapeBuilder.setColor(core::Color::alpha(core::Color::Red, 0.6f));
	_shapeBuilder.cube(glm::vec3(0.0f), glm::vec3(1.0f), flags);
	_shapeRenderer.createOrUpdate(_voxelCursorMesh, _shapeBuilder);
}

void ModifierRenderer::updateSelectionBuffers(const Selections& selections) {
	_shapeBuilder.clear();
	_shapeBuilder.setColor(core::Color::Yellow);
	for (const Selection &selection : selections) {
		_shapeBuilder.aabb(selection.getLowerCorner(), selection.getUpperCorner() + glm::one<glm::ivec3>());
	}
	_shapeRenderer.createOrUpdate(_selectionIndex, _shapeBuilder);
}

void ModifierRenderer::clearBrushMeshes() {
	_volumeRenderer.clear();
}

void ModifierRenderer::updateBrushVolume(int idx, voxel::RawVolume *volume, voxel::Palette *palette) {
	delete _volumeRenderer.setVolume(idx, volume, palette);
	if (volume != nullptr) {
		_volumeRenderer.extractRegion(idx, volume->region());
	}
}

void ModifierRenderer::renderBrushVolume(const video::Camera &camera) {
	if (_volumeRendererCtx.frameBuffer.dimension() != camera.size()) {
		_volumeRendererCtx.shutdown();
		_volumeRendererCtx.init(camera.size());
	}
	while (_volumeRenderer.scheduleExtractions(100)) {
	}
	_volumeRenderer.waitForPendingExtractions();
	_volumeRenderer.update();
	_volumeRenderer.render(_volumeRendererCtx, camera, false);
}

void ModifierRenderer::render(const video::Camera& camera, const glm::mat4& model) {
	const video::ScopedState depthTest(video::State::DepthTest, false);
	const video::ScopedState cullFace(video::State::CullFace, false);
	_shapeRenderer.render(_voxelCursorMesh, camera, model);
	_shapeRenderer.render(_mirrorMeshIndex, camera);
	_shapeRenderer.render(_referencePointMesh, camera, _referencePointModelMatrix);
}

void ModifierRenderer::updateReferencePosition(const glm::ivec3 &pos) {
	const glm::vec3 posAligned((float)pos.x + 0.5f, (float)pos.y + 0.5f, (float)pos.z + 0.5f);
	_referencePointModelMatrix = glm::translate(posAligned);
}

void ModifierRenderer::renderSelection(const video::Camera& camera) {
	_shapeRenderer.render(_selectionIndex, camera);
}

void ModifierRenderer::updateMirrorPlane(math::Axis axis, const glm::ivec3& mirrorPos) {
	if (axis == math::Axis::None) {
		if (_mirrorMeshIndex != -1) {
			_shapeRenderer.deleteMesh(_mirrorMeshIndex);
			_mirrorMeshIndex = -1;
		}
		return;
	}

	const voxel::Region region = sceneMgr().sceneGraph().region();
	const glm::vec4 color = core::Color::alpha(core::Color::LightGray, 0.3f);
	updateShapeBuilderForPlane(_shapeBuilder, region, true, mirrorPos, axis, color);
	_shapeRenderer.createOrUpdate(_mirrorMeshIndex, _shapeBuilder);
}

}
