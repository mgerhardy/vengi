/**
 * @file
 */

/*
MIT License

Copyright (c) 2020 Erik Johansson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include "BinaryGreedyMesher.h"
#include "core/Log.h"
#include "voxel/ChunkMesh.h"
#include "voxel/RawVolume.h"
#include "voxel/Region.h"
#include "voxel/VoxelVertex.h"
#include <stdint.h>
#include <vector>

namespace voxel {

// CS = chunk size (max 62)
static constexpr int CS = 62;

// Padded chunk size
static constexpr int CS_P = CS + 2;
static constexpr int CS_P2 = CS_P * CS_P;
static constexpr int CS_P3 = CS_P * CS_P * CS_P;

struct MeshData {
	std::vector<uint64_t> col_face_masks; // CS_P2 * 6
	std::vector<uint64_t> a_axis_cols;	  // CS_P2
	std::vector<uint64_t> b_axis_cols;	  // CS_P
	std::vector<uint64_t> merged_right;	  // CS_P
	std::vector<uint64_t> merged_forward; // CS_P2
};

CORE_FORCE_INLINE int get_axis_i(const int axis, const int a, const int b, const int c) {
	if (axis == 0)
		return b + (a * CS_P) + (c * CS_P2);
	else if (axis == 1)
		return a + (c * CS_P) + (b * CS_P2);
	return c + (b * CS_P) + (a * CS_P2);
}

// Add checks to this function to skip culling against grass for example
CORE_FORCE_INLINE bool solid_check(const voxel::Voxel &voxel) {
	return !isAir(voxel.getMaterial());
}

static const glm::ivec2 ao_dirs[8] = {
	glm::ivec2(-1, 0),	glm::ivec2(0, -1), glm::ivec2(0, 1),  glm::ivec2(1, 0),
	glm::ivec2(-1, -1), glm::ivec2(-1, 1), glm::ivec2(1, -1), glm::ivec2(1, 1),
};

CORE_FORCE_INLINE int vertexAO(uint8_t side1, uint8_t side2, uint8_t corner) {
	return (side1 && side2) ? 0 : (3 - (side1 + side2 + corner));
}

CORE_FORCE_INLINE bool compare_ao(const std::vector<voxel::Voxel> &voxels, int axis, int forward, int right, int c,
								  int forward_offset, int right_offset) {
	for (const auto &ao_dir : ao_dirs) {
		if (solid_check(voxels[get_axis_i(axis, right + ao_dir[0], forward + ao_dir[1], c)]) !=
			solid_check(
				voxels[get_axis_i(axis, right + right_offset + ao_dir[0], forward + forward_offset + ao_dir[1], c)])) {
			return false;
		}
	}
	return true;
}

CORE_FORCE_INLINE void insert_quad(Mesh &mesh, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4, bool flipped) {
	if (flipped) {
		mesh.addTriangle(v1, v2, v3);
		mesh.addTriangle(v3, v4, v1);
	} else {
		mesh.addTriangle(v1, v2, v4);
		mesh.addTriangle(v4, v2, v3);
	}
}

CORE_FORCE_INLINE uint32_t get_vertex(Mesh &mesh, uint32_t x, uint32_t y, uint32_t z, const voxel::Voxel &voxel,
									  uint32_t norm, uint32_t ao, const glm::ivec3 &translate) {
	VoxelVertex vertex;
	vertex.position = glm::vec3(x + translate.x, y + translate.y, z + translate.z);
	vertex.info = 0;
	vertex.ambientOcclusion = ao;
	vertex.colorIndex = voxel.getColor();
	IndexType index = mesh.addVertex(vertex);
	// mesh.setNormal(index, norm);
	return index;
}

static const uint64_t CULL_MASK = (1ULL << (CS_P - 1));
static const uint64_t BORDER_MASK = (1ULL | (1ULL << (CS_P - 1)));

static void prepareChunk(const voxel::RawVolume &map, std::vector<Voxel> &voxels, const glm::ivec3 &chunkPos,
						 MeshData &mesh) {
	voxel::RawVolume::Sampler sampler(map);
	voxels.reserve(CS_P3);
	sampler.setPosition(chunkPos);
	for (uint32_t y = 1; y < CS_P; y++) {
		voxel::RawVolume::Sampler sampler2 = sampler;
		for (uint32_t x = 1; x < CS_P; x++) {
			voxel::RawVolume::Sampler sampler3 = sampler2;
			for (uint32_t z = 1; z < CS_P; z++) {
				const int index = z + (x * CS_P) + (y * CS_P2);
				voxels[index] = sampler3.voxel();
				sampler3.movePositiveZ();
			}
			sampler2.movePositiveX();
		}
		sampler.movePositiveY();
	}
}

void extractBinaryGreedyMesh(const voxel::RawVolume *volData, const Region &region, ChunkMesh *result,
							 const glm::ivec3 &translate, bool bake_ao, bool optimize) {
	// loop over each chunk of the size CS_P * CS_P * CS_P and extract the mesh for it
	// then merge the mesh into the result

	result->clear();
	const glm::ivec3& offset = region.getLowerCorner();
	result->setOffset(offset);

	MeshData meshData;
	std::vector<voxel::Voxel> voxels;
	prepareChunk(*volData, voxels, region.getLowerCorner(), meshData);

	std::vector<uint64_t> col_face_masks = meshData.col_face_masks;
	std::vector<uint64_t> a_axis_cols = meshData.a_axis_cols;
	std::vector<uint64_t> b_axis_cols = meshData.b_axis_cols;
	std::vector<uint64_t> merged_right = meshData.merged_right;
	std::vector<uint64_t> merged_forward = meshData.merged_forward;
	col_face_masks.reserve(CS_P2 * 6);
	a_axis_cols.reserve(CS_P2);
	b_axis_cols.reserve(CS_P);
	merged_right.reserve(CS_P);
	merged_forward.reserve(CS_P2);
	// Begin culling faces
	auto p = voxels.begin();
	core_memset(a_axis_cols.data(), 0, CS_P2);
	for (int a = 0; a < CS_P; a++) {
		core_memset(b_axis_cols.data(), 0, CS_P * 8);

		for (int b = 0; b < CS_P; b++) {
			uint64_t cb = 0;

			for (int c = 0; c < CS_P; c++) {
				if (solid_check(*p)) {
					a_axis_cols[b + (c * CS_P)] |= 1ULL << a;
					b_axis_cols[c] |= 1ULL << b;
					cb |= 1ULL << c;
				}
				++p;
			}

			// Cull third axis faces
			col_face_masks[a + (b * CS_P) + (4 * CS_P2)] = cb & ~((cb >> 1) | CULL_MASK);
			col_face_masks[a + (b * CS_P) + (5 * CS_P2)] = cb & ~((cb << 1) | 1ULL);
		}

		// Cull second axis faces
		int faceIndex = (a * CS_P) + (2 * CS_P2);
		for (int b = 1; b < CS_P - 1; b++) {
			uint64_t col = b_axis_cols[b];
			col_face_masks[faceIndex + b] = col & ~((col >> 1) | CULL_MASK);
			col_face_masks[faceIndex + b + CS_P2] = col & ~((col << 1) | 1ULL);
		}
	}

	// Cull first axis faces
	for (int a = 1; a < CS_P - 1; a++) {
		int faceIndex = a * CS_P;
		for (int b = 1; b < CS_P - 1; b++) {
			uint64_t col = a_axis_cols[faceIndex + b];

			col_face_masks[faceIndex + b] = col & ~((col >> 1) | CULL_MASK);
			col_face_masks[faceIndex + b + CS_P2] = col & ~((col << 1) | 1ULL);
		}
	}

	// Greedy meshing
	for (uint8_t face = 0; face < 6; face++) {
		int axis = face / 2;
		int air_dir = face % 2 == 0 ? 1 : -1;

		core_memset(merged_forward.data(), 0, CS_P2 * 8);

		for (int forward = 1; forward < CS_P - 1; forward++) {
			uint64_t bits_walking_right = 0;
			int forwardIndex = (forward * CS_P) + (face * CS_P2);

			core_memset(merged_right.data(), 0, CS_P * 8);

			for (int right = 1; right < CS_P - 1; right++) {
				int rightxCS_P = right * CS_P;

				uint64_t bits_here = col_face_masks[forwardIndex + right] & ~BORDER_MASK;
				uint64_t bits_right = right >= CS ? 0 : col_face_masks[forwardIndex + right + 1];
				uint64_t bits_forward = forward >= CS ? 0 : col_face_masks[forwardIndex + right + CS_P];

				uint64_t bits_merging_forward = bits_here & bits_forward & ~bits_walking_right;
				uint64_t bits_merging_right = bits_here & bits_right;

				unsigned long bit_pos;

				uint64_t copy_front = bits_merging_forward;
				while (copy_front) {
#ifdef _MSC_VER
					_BitScanForward64(&bit_pos, copy_front);
#else
					bit_pos = __builtin_ctzll(copy_front);
#endif

					copy_front &= ~(1ULL << bit_pos);

					if (voxels[get_axis_i(axis, right, forward, bit_pos)].isSame(
							voxels[get_axis_i(axis, right, forward + 1, bit_pos)]) &&
						(!bake_ao || compare_ao(voxels, axis, forward, right, bit_pos + air_dir, 1, 0))) {
						merged_forward[(right * CS_P) + bit_pos]++;
					} else {
						bits_merging_forward &= ~(1ULL << bit_pos);
					}
				}

				uint64_t bits_stopped_forward = bits_here & ~bits_merging_forward;
				while (bits_stopped_forward) {
#ifdef _MSC_VER
					_BitScanForward64(&bit_pos, bits_stopped_forward);
#else
					bit_pos = __builtin_ctzll(bits_stopped_forward);
#endif

					bits_stopped_forward &= ~(1ULL << bit_pos);

					const voxel::Voxel &type = voxels[get_axis_i(axis, right, forward, bit_pos)];

					if ((bits_merging_right & (1ULL << bit_pos)) != 0 &&
						(merged_forward[(right * CS_P) + bit_pos] == merged_forward[(right + 1) * CS_P + bit_pos]) &&
						(type.isSame(voxels[get_axis_i(axis, right + 1, forward, bit_pos)])) &&
						(!bake_ao || compare_ao(voxels, axis, forward, right, bit_pos + air_dir, 0, 1))) {
						bits_walking_right |= 1ULL << bit_pos;
						merged_right[bit_pos]++;
						merged_forward[rightxCS_P + bit_pos] = 0;
						continue;
					}

					bits_walking_right &= ~(1ULL << bit_pos);

					const uint8_t mesh_left = right - merged_right[bit_pos];
					const uint8_t mesh_right = right + 1;
					const uint8_t mesh_front = forward - merged_forward[rightxCS_P + bit_pos];
					const uint8_t mesh_back = forward + 1;
					const uint8_t mesh_up = bit_pos + (face % 2 == 0 ? 1 : 0);

					uint8_t ao_LB = 3, ao_RB = 3, ao_RF = 3, ao_LF = 3;
					if (bake_ao) {
						const int c = bit_pos + air_dir;
						const uint8_t ao_F = solid_check(voxels[get_axis_i(axis, right, forward - 1, c)]);
						const uint8_t ao_B = solid_check(voxels[get_axis_i(axis, right, forward + 1, c)]);
						const uint8_t ao_L = solid_check(voxels[get_axis_i(axis, right - 1, forward, c)]);
						const uint8_t ao_R = solid_check(voxels[get_axis_i(axis, right + 1, forward, c)]);

						uint8_t ao_LFC =
							!ao_L && !ao_F && solid_check(voxels[get_axis_i(axis, right - 1, forward - 1, c)]);
						uint8_t ao_LBC =
							!ao_L && !ao_B && solid_check(voxels[get_axis_i(axis, right - 1, forward + 1, c)]);
						uint8_t ao_RFC =
							!ao_R && !ao_F && solid_check(voxels[get_axis_i(axis, right + 1, forward - 1, c)]);
						uint8_t ao_RBC =
							!ao_R && !ao_B && solid_check(voxels[get_axis_i(axis, right + 1, forward + 1, c)]);

						ao_LB = vertexAO(ao_L, ao_B, ao_LBC);
						ao_RB = vertexAO(ao_R, ao_B, ao_RBC);
						ao_RF = vertexAO(ao_R, ao_F, ao_RFC);
						ao_LF = vertexAO(ao_L, ao_F, ao_LFC);
					}

					merged_forward[rightxCS_P + bit_pos] = 0;
					merged_right[bit_pos] = 0;

					uint32_t v1, v2, v3, v4;
					const int meshIndex = 0;
					Mesh &mesh = result->mesh[meshIndex];
					if (face == 0) {
						v1 = get_vertex(mesh, mesh_left, mesh_up, mesh_back, type, face, ao_LB, translate);
						v2 = get_vertex(mesh, mesh_right, mesh_up, mesh_back, type, face, ao_RB, translate);
						v3 = get_vertex(mesh, mesh_right, mesh_up, mesh_front, type, face, ao_RF, translate);
						v4 = get_vertex(mesh, mesh_left, mesh_up, mesh_front, type, face, ao_LF, translate);
					} else if (face == 1) {
						v1 = get_vertex(mesh, mesh_left, mesh_up, mesh_back, type, face, ao_LB, translate);
						v2 = get_vertex(mesh, mesh_left, mesh_up, mesh_front, type, face, ao_LF, translate);
						v3 = get_vertex(mesh, mesh_right, mesh_up, mesh_front, type, face, ao_RF, translate);
						v4 = get_vertex(mesh, mesh_right, mesh_up, mesh_back, type, face, ao_RB, translate);
					} else if (face == 2) {
						v1 = get_vertex(mesh, mesh_up, mesh_back, mesh_left, type, face, ao_LB, translate);
						v2 = get_vertex(mesh, mesh_up, mesh_back, mesh_right, type, face, ao_RB, translate);
						v3 = get_vertex(mesh, mesh_up, mesh_front, mesh_right, type, face, ao_RF, translate);
						v4 = get_vertex(mesh, mesh_up, mesh_front, mesh_left, type, face, ao_LF, translate);
					} else if (face == 3) {
						v1 = get_vertex(mesh, mesh_up, mesh_back, mesh_left, type, face, ao_LB, translate);
						v2 = get_vertex(mesh, mesh_up, mesh_front, mesh_left, type, face, ao_LF, translate);
						v3 = get_vertex(mesh, mesh_up, mesh_front, mesh_right, type, face, ao_RF, translate);
						v4 = get_vertex(mesh, mesh_up, mesh_back, mesh_right, type, face, ao_RB, translate);
					} else if (face == 4) {
						v1 = get_vertex(mesh, mesh_back, mesh_left, mesh_up, type, face, ao_LB, translate);
						v2 = get_vertex(mesh, mesh_back, mesh_right, mesh_up, type, face, ao_RB, translate);
						v3 = get_vertex(mesh, mesh_front, mesh_right, mesh_up, type, face, ao_RF, translate);
						v4 = get_vertex(mesh, mesh_front, mesh_left, mesh_up, type, face, ao_LF, translate);
					} else {
						v1 = get_vertex(mesh, mesh_back, mesh_left, mesh_up, type, face, ao_LB, translate);
						v2 = get_vertex(mesh, mesh_front, mesh_left, mesh_up, type, face, ao_LF, translate);
						v3 = get_vertex(mesh, mesh_front, mesh_right, mesh_up, type, face, ao_RF, translate);
						v4 = get_vertex(mesh, mesh_back, mesh_right, mesh_up, type, face, ao_RB, translate);
					}

					insert_quad(mesh, v1, v2, v3, v4, ao_LB + ao_RF > ao_RB + ao_LF);
				}
			}
		}
	}

	if (optimize) {
		result->optimize();
	}
	result->removeUnusedVertices();
	result->compressIndices();
}

} // namespace voxel
