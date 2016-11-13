/**
 * @file
 */

#pragma once

#include "voxel/polyvox/Voxel.h"
#include "core/Common.h"
#include "voxel/polyvox/Raycast.h"

namespace voxel {

class RawVolume;
class PagedVolume;

namespace shape {

template<class Volume>
void createCirclePlane(Volume& volume, const glm::ivec3& center, int width, int depth, double radius, const Voxel& voxel) {
	const int xRadius = width / 2;
	const int zRadius = depth / 2;
	const double minRadius = std::min(xRadius, zRadius);
	const double ratioX = xRadius / minRadius;
	const double ratioZ = zRadius / minRadius;

	for (int z = -zRadius; z <= zRadius; ++z) {
		for (int x = -xRadius; x <= xRadius; ++x) {
			const double distance = glm::pow(x / ratioX, 2.0) + glm::pow(z / ratioZ, 2.0);
			if (distance > radius) {
				continue;
			}
			const glm::ivec3 pos(center.x + x, center.y, center.z + z);
			volume.setVoxel(pos, voxel);
		}
	}
}

template<class Volume>
void createCube(Volume& volume, const glm::ivec3& center, int width, int height, int depth, const Voxel& voxel) {
	const int heightLow = height / 2;
	const int heightHigh = height - heightLow;
	const int widthLow = width / 2;
	const int widthHigh = width - widthLow;
	const int depthLow = depth / 2;
	const int depthHigh = depth - depthLow;
	for (int x = -widthLow; x < widthHigh; ++x) {
		for (int y = -heightLow; y < heightHigh; ++y) {
			for (int z = -depthLow; z < depthHigh; ++z) {
				const glm::ivec3 pos(center.x + x, center.y + y, center.z + z);
				volume.setVoxel(pos, voxel);
			}
		}
	}
}

template<class Volume>
void createCubeNoCenter(Volume& volume, const glm::ivec3& pos, int width, int height, int depth, const Voxel& voxel) {
	const int w = glm::abs(width);
	const int h = glm::abs(height);
	const int d = glm::abs(depth);

	const int sw = width / w;
	const int sh = height / h;
	const int sd = depth / d;

	glm::ivec3 p = pos;
	for (int x = 0; x < w; ++x) {
		p.y = pos.y;
		for (int y = 0; y < h; ++y) {
			p.z = pos.z;
			for (int z = 0; z < d; ++z) {
				volume.setVoxel(p, voxel);
				p.z += sd;
			}
			p.y += sh;
		}
		p.x += sw;
	}
}

template<class Volume>
void createPlane(Volume& volume, const glm::ivec3& center, int width, int depth, const Voxel& voxel) {
	createCube(volume, center, width, 1, depth, voxel);
}

template<class Volume>
glm::ivec3 createL(Volume& volume, const glm::ivec3& pos, int width, int depth, int height, int thickness, const Voxel& voxel) {
	glm::ivec3 p = pos;
	if (width != 0) {
		createCubeNoCenter(volume, p, width, thickness, thickness, voxel);
		p.x += width;
		createCubeNoCenter(volume, p, thickness, height, thickness, voxel);
		p.x += thickness / 2;
		p.z += thickness / 2;
	} else if (depth != 0) {
		createCubeNoCenter(volume, p, thickness, thickness, depth, voxel);
		p.z += depth;
		createCubeNoCenter(volume, p, thickness, height, thickness, voxel);
		p.x += thickness / 2;
		p.z += thickness / 2;
	} else {
		core_assert(false);
	}
	p.y += height;
	return p;
}

template<class Volume>
void createEllipse(Volume& volume, const glm::ivec3& pos, int width, int height, int depth, const Voxel& voxel) {
	const int heightLow = height / 2;
	const int heightHigh = height - heightLow;
	const double minDimension = std::min(width, depth);
	const double adjustedMinRadius = minDimension / 2.0;
	const double heightFactor = heightLow / adjustedMinRadius;
	for (int y = -heightLow; y <= heightHigh; ++y) {
		const double percent = glm::abs(y / heightFactor);
		const double circleRadius = glm::pow(adjustedMinRadius + 0.5, 2.0) - glm::pow(percent, 2.0);
		const glm::ivec3 planePos(pos.x, pos.y + y, pos.z);
		createCirclePlane(volume, planePos, width, depth, circleRadius, voxel);
	}
}

template<class Volume>
void createCone(Volume& volume, const glm::ivec3& pos, int width, int height, int depth, const Voxel& voxel) {
	const int heightLow = height / 2;
	const int heightHigh = height - heightLow;
	const double minDimension = std::min(width, depth);
	const double minRadius = minDimension / 2.0;
	for (int y = -heightLow; y <= heightHigh; ++y) {
		const double percent = 1.0 - ((y + heightLow) / double(height));
		const double circleRadius = glm::pow(percent * minRadius, 2.0);
		const glm::ivec3 planePos(pos.x, pos.y + y, pos.z);
		createCirclePlane(volume, planePos, width, depth, circleRadius, voxel);
	}
}

template<class Volume>
void createDome(Volume& volume, const glm::ivec3& pos, int width, int height, int depth, const Voxel& voxel) {
	const int heightLow = height / 2;
	const int heightHigh = height - heightLow;
	const double minDimension = std::min(width, depth);
	const double minRadius = minDimension / 2.0;
	const double heightFactor = height / (minDimension - 1.0) / 2.0;
	for (int y = -heightLow; y <= heightHigh; ++y) {
		const double percent = glm::abs((y + heightLow) / heightFactor);
		const double circleRadius = glm::pow(minRadius, 2.0) - glm::pow(percent, 2.0);
		const glm::ivec3 planePos(pos.x, pos.y + y, pos.z);
		createCirclePlane(volume, planePos, width, depth, circleRadius, voxel);
	}
}

// http://members.chello.at/~easyfilter/bresenham.html
extern void createLine(PagedVolume& volume, const glm::ivec3& start, const glm::ivec3& end, const Voxel& voxel);
extern void createLine(RawVolume& volume, const glm::ivec3& start, const glm::ivec3& end, const Voxel& voxel);

}
}
