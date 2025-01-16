/**
 * @file
 */

#pragma once

#include "PlanegridShader.h"
#include "core/Log.h"
#include "render/ShapeRenderer.h"
#include "video/ShapeBuilder.h"
#include "math/AABB.h"
#include "video/gl/GLTypes.h"

namespace video {
class Video;
}

namespace render {

/**
 * @brief Renders a grid or bounding box for a given region
 *
 * @note Also hides sides of the grid that would occlude the view to the inside
 *
 * @todo Might be a good idea to implement this as a two sides plane view with backface culling
 */
class GridRenderer {
protected:
	video::ShapeBuilder _shapeBuilder;
	render::ShapeRenderer _shapeRenderer;

	shader::PlanegridShader &_planeShader;
	alignas(16) mutable shader::PlanegridData::UniformblockData _uniformBlockData;
	mutable shader::PlanegridData _uniformBlock;
	video::Id _planeVAO = video::InvalidId;

	math::AABB<float> _aabb;

	int32_t _aabbMeshIndex = -1;
	int32_t _gridMeshIndexXYNear = -1;
	int32_t _gridMeshIndexXYFar = -1;
	int32_t _gridMeshIndexXZNear = -1;
	int32_t _gridMeshIndexXZFar = -1;
	int32_t _gridMeshIndexYZNear = -1;
	int32_t _gridMeshIndexYZFar = -1;
	int32_t _arrow = -1;
	int32_t _plane = -1;
	void createForwardArrow(const math::AABB<float> &aabb);
	void createPlane();
	int _planeGridSize = -1;
	int _resolution = -1;
	bool _renderAABB;
	bool _renderGrid;
	bool _renderPlane;
	bool _dirty = false;
	bool _dirtyPlane = true;
public:
	GridRenderer(bool renderAABB = false, bool renderGrid = true, bool renderPlane = false);

	bool setGridResolution(int resolution);
	int gridResolution() const;

	/**
	 * @param aabb The region to do the plane culling with
	 */
	void render(const video::Camera& camera, const math::AABB<float>& aabb);
	void renderForwardArrow(const video::Camera& camera);
	void renderPlane(const video::Camera &camera);

	void setPlaneGridSize(int planeGridSize);
	int planeGridSize() const;

	bool isRenderAABB() const;
	void setRenderAABB(bool renderAABB);

	bool isRenderPlane() const;
	void setRenderPlane(bool renderPlane);

	bool isRenderGrid() const;
	void setRenderGrid(bool renderGrid);

	/**
	 * @brief Update the internal render buffers for the new region.
	 * @param region The region to render the grid for
	 */
	void update(const math::AABB<float> &region);
	void clear();
	void setColor(const glm::vec4 &color);

	/**
	 * @sa shutdown()
	 */
	bool init();

	void shutdown();
};

inline void GridRenderer::setPlaneGridSize(int planeGridSize) {
	if (_planeGridSize == planeGridSize) {
		return;
	}
	_planeGridSize = planeGridSize;
	_dirtyPlane = true;
}

inline int GridRenderer::planeGridSize() const {
	return _planeGridSize;
}

inline bool GridRenderer::isRenderPlane() const {
	return _renderPlane;
}

inline void GridRenderer::setRenderPlane(bool renderPlane) {
	if (_renderPlane == renderPlane) {
		return;
	}
	_renderPlane = renderPlane;
}

inline bool GridRenderer::isRenderAABB() const {
	return _renderAABB;
}

inline bool GridRenderer::isRenderGrid() const {
	return _renderGrid;
}

inline void GridRenderer::setRenderAABB(bool renderAABB) {
	if (_renderAABB == renderAABB) {
		return;
	}
	_renderAABB = renderAABB;
	_dirty = true;
}

inline void GridRenderer::setRenderGrid(bool renderGrid) {
	if (_renderGrid == renderGrid) {
		return;
	}
	_renderGrid = renderGrid;
	_dirty = true;
}

}
