/**
 * @file
 */

#pragma once

#include "voxel/RawVolume.h"
#include "VolumeMerger.h"
#include "core/Common.h"
#include "voxelutil/VolumeVisitor.h"

namespace voxelutil {

/**
 * @brief Will skip air voxels on volume cropping
 */
struct CropSkipEmpty {
	inline bool operator() (const voxel::Voxel& voxel) const {
		return isAir(voxel.getMaterial());
	}
};

/**
 * @brief Resizes a volume to cut off empty parts
 */
template<class CropSkipCondition = CropSkipEmpty>
voxel::RawVolume* cropVolume(const voxel::RawVolume* volume, const glm::ivec3& mins, const glm::ivec3& maxs, CropSkipCondition condition = CropSkipCondition()) {
	core_trace_scoped(CropRawVolume);
	const voxel::Region newRegion(mins, maxs);
	if (!newRegion.isValid()) {
		return nullptr;
	}
	voxel::RawVolume* newVolume = new voxel::RawVolume(newRegion);
	voxelutil::mergeVolumes(newVolume, volume, newRegion, voxel::Region(mins, maxs));
	return newVolume;
}

/**
 * @brief Resizes a volume to cut off empty parts
 */
template<class CropSkipCondition = CropSkipEmpty>
voxel::RawVolume *cropVolume(const voxel::RawVolume *volume) {
	core_trace_scoped(CropRawVolume);
	glm::ivec3 newMins((std::numeric_limits<int>::max)() / 2);
	glm::ivec3 newMaxs((std::numeric_limits<int>::min)() / 2);
	if (visitVolume(
			*volume,
			[&newMins, &newMaxs](int x, int y, int z, const voxel::Voxel &) {
				newMins.x = core_min(newMins.x, x);
				newMins.y = core_min(newMins.y, y);
				newMins.z = core_min(newMins.z, z);

				newMaxs.x = core_max(newMaxs.x, x);
				newMaxs.y = core_max(newMaxs.y, y);
				newMaxs.z = core_max(newMaxs.z, z);
			},
			SkipEmpty()) == 0) {
		return nullptr;
	}
	return cropVolume(volume, newMins, newMaxs);
}
}
