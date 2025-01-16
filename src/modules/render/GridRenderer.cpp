/**
 * @file
 */

#include "GridRenderer.h"
#include "PlanegridShader.h"
#include "core/Color.h"
#include "core/GLM.h"
#include "core/GLMConst.h"
#include "core/Log.h"
#include "core/Trace.h"
#include "math/AABB.h"
#include "math/Plane.h"
#include "video/Buffer.h"
#include "video/Camera.h"
#include "video/Renderer.h"
#include "video/RendererInterface.h"
#include "video/ScopedState.h"
#include "video/Shader.h"
#include "video/Types.h"
#include "video/gl/GLTypes.h"

namespace render {

GridRenderer::GridRenderer(bool renderAABB, bool renderGrid, bool renderPlane)
	: _planeShader(shader::PlanegridShader::getInstance()), _renderAABB(renderAABB), _renderGrid(renderGrid),
	  _renderPlane(renderPlane) {
}

bool GridRenderer::init() {
	if (!_shapeRenderer.init()) {
		Log::error("Failed to initialize the shape renderer");
		return false;
	}

	if (!_planeShader.setup()) {
		Log::error("Failed to setup color shader");
		return false;
	}
	core_assert_always(_uniformBlock.create(_uniformBlockData));
	core_assert_always(_planeShader.setUniformblock(_uniformBlock.getUniformblockUniformBuffer()));
	_planeVAO = video::genVertexArray();

	return true;
}

bool GridRenderer::setGridResolution(int resolution) {
	if (resolution < 1) {
		return false;
	}
	if (_resolution == resolution) {
		return false;
	}
	_resolution = resolution;
	_dirty = true;
	_dirtyPlane = true;
	return true;
}

int GridRenderer::gridResolution() const {
	return _resolution;
}

void GridRenderer::setColor(const glm::vec4 &color) {
	if (_shapeBuilder.setColor(color)) {
		_dirty = true;
	}
}

void GridRenderer::createForwardArrow(const math::AABB<float> &aabb) {
	if (!aabb.isValid() || aabb.isEmpty()) {
		return;
	}
	_shapeBuilder.clear();
	const float arrowSize = 10.0f;
	const float forward = glm::forward().z * arrowSize;
	const float left = glm::left().x * arrowSize;
	const float right = glm::right().x * arrowSize;
	const float arrowX = aabb.getCenterX();
	const float arrowY = aabb.getLowerY();
	const float arrowZ = aabb.getLowerZ();
	const glm::vec3 point1{arrowX + left, arrowY, arrowZ + forward};
	const glm::vec3 point2{arrowX, arrowY, arrowZ + 2.0f * forward};
	const glm::vec3 point3{arrowX + right, arrowY, arrowZ + forward};
	_shapeBuilder.arrow(point1, point2, point3);
	_shapeRenderer.createOrUpdate(_arrow, _shapeBuilder);
	_shapeRenderer.hide(_arrow, true);
}

void GridRenderer::createPlane() {
	_shapeBuilder.clear();
	for (int i = -_planeGridSize; i <= _planeGridSize; ++i) {
		if (glm::abs(i) % 100 == 0) {
			_shapeBuilder.setColor(core::Color::DarkGray());
		} else if (glm::abs(i) % 10 == 0) {
			_shapeBuilder.setColor(core::Color::Gray());
		} else {
			_shapeBuilder.setColor(core::Color::LightGray());
		}
		for (int dir = 0; dir < 2; dir++) {
			glm::vec3 start(dir ? -_planeGridSize : i, 0.f, dir ? i : -_planeGridSize);
			glm::vec3 end(dir ? _planeGridSize : i, 0.f, dir ? i : _planeGridSize);
			_shapeBuilder.line(start, end);
		}
	}
	_shapeRenderer.createOrUpdate(_plane, _shapeBuilder);
	_shapeRenderer.hide(_plane, true);
}

void GridRenderer::update(const math::AABB<float> &aabb) {
	if (!aabb.isValid()) {
		return;
	}
	if (!_dirty && _aabb == aabb) {
		return;
	}
	const float thickness = 1.0f;
	_aabb = aabb;
	_shapeBuilder.clear();
	_shapeBuilder.aabb(aabb, false, 1.0f, thickness);
	_shapeRenderer.createOrUpdate(_aabbMeshIndex, _shapeBuilder);

	_shapeBuilder.clear();
	_shapeBuilder.aabbGridXY(aabb, false, (float)_resolution, thickness);
	_shapeRenderer.createOrUpdate(_gridMeshIndexXYFar, _shapeBuilder);

	_shapeBuilder.clear();
	_shapeBuilder.aabbGridXZ(aabb, false, (float)_resolution, thickness);
	_shapeRenderer.createOrUpdate(_gridMeshIndexXZFar, _shapeBuilder);

	_shapeBuilder.clear();
	_shapeBuilder.aabbGridYZ(aabb, false, (float)_resolution, thickness);
	_shapeRenderer.createOrUpdate(_gridMeshIndexYZFar, _shapeBuilder);

	_shapeBuilder.clear();
	_shapeBuilder.aabbGridXY(aabb, true, (float)_resolution, thickness);
	_shapeRenderer.createOrUpdate(_gridMeshIndexXYNear, _shapeBuilder);

	_shapeBuilder.clear();
	_shapeBuilder.aabbGridXZ(aabb, true, (float)_resolution, thickness);
	_shapeRenderer.createOrUpdate(_gridMeshIndexXZNear, _shapeBuilder);

	_shapeBuilder.clear();
	_shapeBuilder.aabbGridYZ(aabb, true, (float)_resolution, thickness);
	_shapeRenderer.createOrUpdate(_gridMeshIndexYZNear, _shapeBuilder);

	createForwardArrow(aabb);

	_dirty = false;
}

void GridRenderer::clear() {
	_shapeBuilder.clear();
	_dirty = false;
}

void GridRenderer::render(const video::Camera &camera, const math::AABB<float> &aabb) {
	core_trace_scoped(GridRendererRender);

	if (_dirty) {
		update(aabb);
	}

	_shapeRenderer.hide(_aabbMeshIndex, !_renderAABB);
	_shapeRenderer.hide(_plane, true);
	if (_renderGrid && aabb.isValid()) {
		const glm::vec3 &center = aabb.getCenter();
		const glm::vec3 &halfWidth = aabb.getWidth() / 2.0f;
		const math::Plane planeLeft(glm::left(), center + glm::left() * halfWidth);
		const math::Plane planeRight(glm::right(), center + glm::right() * halfWidth);
		const math::Plane planeBottom(glm::down(), center + glm::down() * halfWidth);
		const math::Plane planeTop(glm::up(), center + glm::up() * halfWidth);
		const math::Plane planeNear(glm::forward(), center + glm::forward() * halfWidth);
		const math::Plane planeFar(glm::backward(), center + glm::backward() * halfWidth);

		if (camera.mode() == video::CameraMode::Perspective) {
			const glm::vec3 &eye = camera.eye();
			_shapeRenderer.hide(_gridMeshIndexXYFar, !planeFar.isBackSide(eye));
			_shapeRenderer.hide(_gridMeshIndexXYNear, !planeNear.isBackSide(eye));
			_shapeRenderer.hide(_gridMeshIndexXZFar, !planeTop.isBackSide(eye));
			_shapeRenderer.hide(_gridMeshIndexXZNear, !planeBottom.isBackSide(eye));
			_shapeRenderer.hide(_gridMeshIndexYZFar, !planeRight.isBackSide(eye));
			_shapeRenderer.hide(_gridMeshIndexYZNear, !planeLeft.isBackSide(eye));
		} else {
			const glm::vec3 &viewDirection = -camera.forward();
			_shapeRenderer.hide(_gridMeshIndexXYFar, glm::dot(viewDirection, planeFar.norm()) > 0);
			_shapeRenderer.hide(_gridMeshIndexXYNear, glm::dot(viewDirection, planeNear.norm()) > 0);
			_shapeRenderer.hide(_gridMeshIndexXZFar, glm::dot(viewDirection, planeTop.norm()) > 0);
			_shapeRenderer.hide(_gridMeshIndexXZNear, glm::dot(viewDirection, planeBottom.norm()) > 0);
			_shapeRenderer.hide(_gridMeshIndexYZFar, glm::dot(viewDirection, planeRight.norm()) > 0);
			_shapeRenderer.hide(_gridMeshIndexYZNear, glm::dot(viewDirection, planeLeft.norm()) > 0);
		}
	} else {
		_shapeRenderer.hide(_gridMeshIndexXYFar, true);
		_shapeRenderer.hide(_gridMeshIndexXYNear, true);
		_shapeRenderer.hide(_gridMeshIndexXZFar, true);
		_shapeRenderer.hide(_gridMeshIndexXZNear, true);
		_shapeRenderer.hide(_gridMeshIndexYZFar, true);
		_shapeRenderer.hide(_gridMeshIndexYZNear, true);
	}
	_shapeRenderer.renderAll(camera);
}

void GridRenderer::renderPlane(const video::Camera &camera) {
	if (!_renderPlane) {
		return;
	}

	video::ScopedState facecull(video::State::CullFace, false);
	video::ScopedState depthdepth(video::State::DepthTest, true);
	video::ScopedShader scopedShader(_planeShader);
	video::bindVertexArray(_planeVAO);

	_uniformBlockData.cameraPos = camera.eye();
	_uniformBlockData.proj = camera.projectionMatrix();
	_uniformBlockData.view = camera.viewMatrix();
	// video::bindBuffer(video::BufferType::UniformBuffer, _uniformBlock.getUniformblockUniformBuffer().handle());
	core_assert_always(_uniformBlock.update(_uniformBlockData));
	core_assert_always(_planeShader.setUniformblock(_uniformBlock.getUniformblockUniformBuffer()));

	video::drawArrays(video::Primitive::TriangleStrip, 4);
	video::bindVertexArray(video::InvalidId);
}

void GridRenderer::renderForwardArrow(const video::Camera &camera) {
	_shapeRenderer.hide(_arrow, false);
	video::ScopedState cull(video::State::CullFace, false);
	_shapeRenderer.render(_arrow, camera);
	_shapeRenderer.hide(_arrow, true);
}

void GridRenderer::shutdown() {
	_aabbMeshIndex = -1;
	_gridMeshIndexXYNear = -1;
	_gridMeshIndexXYFar = -1;
	_gridMeshIndexXZNear = -1;
	_gridMeshIndexXZFar = -1;
	_gridMeshIndexYZNear = -1;
	_gridMeshIndexYZFar = -1;
	_arrow = -1;
	_plane = -1;
	_shapeRenderer.shutdown();
	_shapeBuilder.shutdown();
	_planeShader.shutdown();
	_uniformBlock.shutdown();
	video::deleteVertexArray(_planeVAO);
}

} // namespace render
