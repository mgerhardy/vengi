/**
 * @file
 */

#pragma once

#include "core/RGBA.h"
#include "image/Image.h"
#include "math/Math.h"
#include "video/Types.h"

namespace voxelformat {

struct Tri {
	glm::vec3 vertices[3] {};
	glm::vec2 uv[3] {};
	const image::Image* texture = nullptr;
	core::RGBA color[3]{core::RGBA(0, 0, 0), core::RGBA(0, 0, 0), core::RGBA(0, 0, 0)};
	image::TextureWrap wrapS = image::TextureWrap::Repeat;
	image::TextureWrap wrapT = image::TextureWrap::Repeat;

	constexpr glm::vec2 centerUV() const {
		return (uv[0] + uv[1] + uv[2]) / 3.0f;
	}

	constexpr glm::vec3 center() const {
		return (vertices[0] + vertices[1] + vertices[2]) / 3.0f;
	}

	glm::vec3 calculateBarycentric(const glm::vec3 &pos) const;

	/**
	 * @return @c false if the given position is not within the triangle area. The value of uv should not be used in this case.
	 */
	[[nodiscard]] bool calcUVs(const glm::vec3 &pos, glm::vec2& uv) const;
	/**
	 * @return @c true if the triangle is lying flat on one axis
	 */
	bool flat() const;
	glm::vec3 normal() const;
	float area() const;
	glm::vec3 mins() const;
	glm::vec3 maxs() const;
	glm::ivec3 roundedMins() const;
	glm::ivec3 roundedMaxs() const;
	core::RGBA colorAt(const glm::vec2 &uv) const;
	core::RGBA centerColor() const;

	// Sierpinski gasket with keeping the middle
	void subdivide(Tri out[4]) const;
};

} // namespace voxelformat
