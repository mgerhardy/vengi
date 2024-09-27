/**
 * @file
 *
 * https://github.com/cgerikj/binary-greedy-meshing
 */

#pragma once

#include <glm/fwd.hpp>

namespace palette {
class Palette;
}

namespace voxel {

class RawVolume;
class Region;
struct ChunkMesh;

void extractBinaryGreedyMesh(const voxel::RawVolume *volData, const Region &region, ChunkMesh *result,
							 const glm::ivec3 &translate, bool ambientOcclusion = true, bool optimize = false);

} // namespace voxel
