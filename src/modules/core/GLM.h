/**
 * @file
 */

#pragma once

#include "core/GLMConst.h"
#include "core/Assert.h"
#include "core/Common.h"
#include "core/Hash.h"
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/common.hpp>

namespace glm {

// these are here if you use sse vec types and you still need padded
// vector structs in non see situations
typedef glm::vec<3, float, glm::highp> vec3h;
typedef glm::vec<2, int, glm::highp> ivec2h;
typedef glm::vec<3, uint8_t, glm::highp> u8vec3h;

#define glm_assert_vec3(vec) \
	core_assert_msg(!glm::isnan((vec).x), "x is nan"); \
	core_assert_msg(!glm::isnan((vec).y), "y is nan"); \
	core_assert_msg(!glm::isnan((vec).z), "z is nan"); \
	core_assert_msg(!glm::isinf((vec).x), "x is inf"); \
	core_assert_msg(!glm::isinf((vec).y), "y is inf"); \
	core_assert_msg(!glm::isinf((vec).z), "z is inf");

#define glm_assert_vec4(vec) \
	core_assert_msg(!glm::isnan((vec).x), "x is nan"); \
	core_assert_msg(!glm::isnan((vec).y), "y is nan"); \
	core_assert_msg(!glm::isnan((vec).z), "z is nan"); \
	core_assert_msg(!glm::isnan((vec).w), "w is nan"); \
	core_assert_msg(!glm::isinf((vec).x), "x is inf"); \
	core_assert_msg(!glm::isinf((vec).y), "y is inf"); \
	core_assert_msg(!glm::isinf((vec).z), "z is inf"); \
	core_assert_msg(!glm::isinf((vec).w), "w is nan");

/**
 * Calculate the line segment PaPb that is the shortest route between
 * two lines P1P2 and P3P4. Calculate also the values of @c mua and @c mub where
 *  Pa = P1 + mua (P2 - P1)
 *  Pb = P3 + mub (P4 - P3)
 * @return @c false if no solution exists.
 */
bool intersectLines(const glm::vec3& p1,
		const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4,
		glm::vec3& pa, glm::vec3& pb, float *pmua = nullptr,
		float *pmub = nullptr);

bool intersectTriangleAABB(const glm::vec3 &center, const glm::vec3 &halfSize, glm::vec3 v0, glm::vec3 v1,
						   glm::vec3 v2);

vec3 transform(const mat4& mat, const vec3& v);

vec3 rotate(const mat4& mat, const vec3& v);

vec3 project(const mat4& m, const vec3& p);

glm::mat3x4 invert(const glm::mat3x4& o);

mat3x4 operator*(const mat3x4& lhs, const mat3x4 &o);

CORE_FORCE_INLINE constexpr void hash_combine(uint32_t &seed, uint32_t hash) {
	hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= hash;
}

template<typename T>
struct hash {};

template<typename T, glm::qualifier Q>
struct hash<glm::vec<3, T, Q>> {
constexpr uint32_t operator()(const glm::vec<3, T, Q>& v) const {
	uint32_t seed = 0u;
	hash_combine(seed, core::hash((const void*)&v.x, (int)sizeof(v.x)));
	hash_combine(seed, core::hash((const void*)&v.y, (int)sizeof(v.y)));
	hash_combine(seed, core::hash((const void*)&v.z, (int)sizeof(v.z)));
	return seed;
}
};

template<typename T, glm::qualifier Q>
struct hash<glm::vec<4, T, Q>> {
constexpr uint32_t operator()(const glm::vec<4, T, Q>& v) const {
	uint32_t seed = 0u;
	hash_combine(seed, core::hash((const void*)&v.x, (int)sizeof(v.x)));
	hash_combine(seed, core::hash((const void*)&v.y, (int)sizeof(v.y)));
	hash_combine(seed, core::hash((const void*)&v.z, (int)sizeof(v.z)));
	hash_combine(seed, core::hash((const void*)&v.w, (int)sizeof(v.w)));
	return seed;
}
};

}
