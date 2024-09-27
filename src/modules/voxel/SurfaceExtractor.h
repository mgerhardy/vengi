/**
 * @file
 */

#pragma once

#include "math/Math.h"

namespace palette {
class Palette;
}

namespace voxel {
class RawVolume;
class Region;
struct ChunkMesh;

enum class SurfaceExtractionType { Cubic, MarchingCubes, Binary, Max };

struct SurfaceExtractionContext {
	SurfaceExtractionContext(const RawVolume *_volume, const palette::Palette &_palette, const Region &_region,
							 ChunkMesh &_mesh, const glm::ivec3 &_translate, SurfaceExtractionType _type,
							 bool _mergeQuads, bool _reuseVertices, bool _ambientOcclusion, bool _optimize)
		: volume(_volume), palette(_palette), region(_region), mesh(_mesh), translate(_translate), type(_type),
		  mergeQuads(_mergeQuads), reuseVertices(_reuseVertices), ambientOcclusion(_ambientOcclusion), optimize(_optimize) {
	}
	const RawVolume *volume;
	const palette::Palette &palette; // used only for MarchingCubes
	const Region &region;
	ChunkMesh &mesh;
	const glm::ivec3 translate;
	const SurfaceExtractionType type;
	const bool mergeQuads;		 // used only for Cubic
	const bool reuseVertices;	 // used only for Cubic
	const bool ambientOcclusion; // used only for Cubic
	const bool optimize;
};

SurfaceExtractionContext buildBinaryContext(const RawVolume *volume, const Region &region, ChunkMesh &mesh,
										   const glm::ivec3 &translate, bool mergeQuads, bool reuseVertices,
										   bool ambientOcclusion, bool optimize);
SurfaceExtractionContext buildCubicContext(const RawVolume *volume, const Region &region, ChunkMesh &mesh,
										   const glm::ivec3 &translate = glm::ivec3(0), bool mergeQuads = true,
										   bool reuseVertices = true, bool ambientOcclusion = true, bool optimize = false);
SurfaceExtractionContext buildMarchingCubesContext(const RawVolume *volume, const Region &region, ChunkMesh &mesh,
												   const palette::Palette &palette, bool optimize = false);

void extractSurface(SurfaceExtractionContext &ctx);

voxel::SurfaceExtractionContext createContext(voxel::SurfaceExtractionType type, const voxel::RawVolume *volume,
											  const voxel::Region &region, const palette::Palette &palette,
											  voxel::ChunkMesh &mesh, const glm::ivec3 &translate,
											  bool mergeQuads = true, bool reuseVertices = true,
											  bool ambientOcclusion = true, bool optimize = false);

} // namespace voxel
