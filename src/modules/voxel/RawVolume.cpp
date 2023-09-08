/**
 * @file
 */

#include "RawVolume.h"
#include "core/Assert.h"
#include "core/StandardLib.h"
#include <glm/common.hpp>
#include <limits>

namespace voxel {

size_t RawVolume::size(const Region &region) {
	const size_t w = region.getWidthInVoxels();
	const size_t h = region.getHeightInVoxels();
	const size_t d = region.getDepthInVoxels();
	const size_t size = w * h * d * sizeof(Voxel);
	return size;
}

RawVolume::RawVolume(const Region& regValid) :
		_region(regValid) {
	//Create a volume of the right size.
	initialise(regValid);
}

RawVolume::RawVolume(const RawVolume* copy) :
		_region(copy->region()) {
	setBorderValue(copy->borderValue());
	const size_t size = width() * height() * depth() * sizeof(Voxel);
	_data = (Voxel*)core_malloc(size);
	_borderVoxel = copy->_borderVoxel;
	core_memcpy((void*)_data, (void*)copy->_data, size);
}

RawVolume::RawVolume(const RawVolume& copy) :
		_region(copy.region()) {
	setBorderValue(copy.borderValue());
	const size_t size = width() * height() * depth() * sizeof(Voxel);
	_data = (Voxel*)core_malloc(size);
	_borderVoxel = copy._borderVoxel;
	core_memcpy((void*)_data, (void*)copy._data, size);
}

static inline voxel::Region accumulate(const core::DynamicArray<Region>& regions) {
	voxel::Region r = voxel::Region::InvalidRegion;
	for (const Region &region : regions) {
		if (r.isValid()) {
			r.accumulate(region);
		} else {
			r = region;
		}
	}
	return r;
}

RawVolume::RawVolume(const RawVolume &src, const core::DynamicArray<Region> &copyRegions)
	: _region(accumulate(copyRegions)) {
	_region.cropTo(src.region());
	setBorderValue(src.borderValue());
	initialise(_region);

	for (Region copyRegion : copyRegions) {
		copyRegion.cropTo(_region);
		RawVolume::Sampler destSampler(*this);
		RawVolume::Sampler srcSampler(src);
		for (int32_t x = copyRegion.getLowerX(); x <= copyRegion.getUpperX(); ++x) {
			for (int32_t y = copyRegion.getLowerY(); y <= copyRegion.getUpperY(); ++y) {
				srcSampler.setPosition(x, y, copyRegion.getLowerZ());
				destSampler.setPosition(x, y, copyRegion.getLowerZ());
				for (int32_t z = copyRegion.getLowerZ(); z <= copyRegion.getUpperZ(); ++z) {
					destSampler.setVoxel(srcSampler.voxel());
					destSampler.movePositiveZ();
					srcSampler.movePositiveZ();
				}
			}
		}
	}
}

RawVolume::RawVolume(const RawVolume& src, const Region& region, bool *onlyAir) : _region(region) {
	setBorderValue(src.borderValue());
	if (!src.region().containsRegion(_region)) {
		_region.cropTo(src._region);
	}
	const size_t size = width() * height() * depth() * sizeof(Voxel);
	_data = (Voxel *)core_malloc(size);
	if (src.region() == _region) {
		core_memcpy((void *)_data, (void *)src._data, size);
		if (onlyAir) {
			*onlyAir = false;
		}
	} else {
		if (onlyAir) {
			*onlyAir = true;
		}
		const glm::ivec3 &tgtMins = _region.getLowerCorner();
		const glm::ivec3 &tgtMaxs = _region.getUpperCorner();
		const glm::ivec3 &srcMins = src._region.getLowerCorner();
		const int tgtYStride = _region.getWidthInVoxels();
		const int tgtZStride = _region.getWidthInVoxels() * _region.getHeightInVoxels();
		const int srcYStride = src._region.getWidthInVoxels();
		const int srcZStride = src._region.getWidthInVoxels() * src._region.getHeightInVoxels();
		for (int x = tgtMins.x; x <= tgtMaxs.x; ++x) {
			const int32_t tgtXPos = x - tgtMins.x;
			const int32_t srcXPos = x - srcMins.x;
			for (int y = tgtMins.y; y <= tgtMaxs.y; ++y) {
				const int32_t tgtYPos = y - tgtMins.y;
				const int32_t srcYPos = y - srcMins.y;
				const int tgtStrideLocal = tgtXPos + tgtYPos * tgtYStride;
				const int srcStrideLocal = srcXPos + srcYPos * srcYStride;
				for (int z = tgtMins.z; z <= tgtMaxs.z; ++z) {
					const int32_t tgtZPos = z - tgtMins.z;
					const int32_t srcZPos = z - srcMins.z;
					const int tgtindex = tgtStrideLocal + tgtZPos * tgtZStride;
					const int srcindex = srcStrideLocal + srcZPos * srcZStride;
					_data[tgtindex] = src._data[srcindex];
					if (onlyAir && !voxel::isAir(_data[tgtindex].getMaterial())) {
						*onlyAir = false;
						onlyAir = nullptr;
					}
				}
			}
		}
	}
}

RawVolume::RawVolume(RawVolume&& move) noexcept {
	_data = move._data;
	move._data = nullptr;
	_region = move._region;
}

RawVolume::RawVolume(const Voxel* data, const voxel::Region& region) {
	initialise(region);
	const size_t size = width() * height() * depth() * sizeof(Voxel);
	core_memcpy((void*)_data, (const void*)data, size);
}

RawVolume::RawVolume(Voxel* data, const voxel::Region& region) :
		_region(region), _data(data) {
	core_assert_msg(width() > 0, "Volume width must be greater than zero.");
	core_assert_msg(height() > 0, "Volume height must be greater than zero.");
	core_assert_msg(depth() > 0, "Volume depth must be greater than zero.");
}

RawVolume::~RawVolume() {
	core_free(_data);
	_data = nullptr;
}

Voxel* RawVolume::copyVoxels() const {
	const size_t size = width() * height() * depth() * sizeof(Voxel);
	Voxel* rawCopy = (Voxel*)core_malloc(size);
	core_memcpy((void*)rawCopy, (void*)_data, size);
	return rawCopy;
}

/**
 * This version of the function is provided so that the wrap mode does not need
 * to be specified as a template parameter, as it may be confusing to some users.
 * @param x The @c x position of the voxel
 * @param y The @c y position of the voxel
 * @param z The @c z position of the voxel
 * @return The voxel value
 */
const Voxel& RawVolume::voxel(int32_t x, int32_t y, int32_t z) const {
	if (_region.containsPoint(x, y, z)) {
		const int32_t iLocalXPos = x - _region.getLowerX();
		const int32_t iLocalYPos = y - _region.getLowerY();
		const int32_t iLocalZPos = z - _region.getLowerZ();

		return _data[iLocalXPos + iLocalYPos * width() + iLocalZPos * width() * height()];
	}
	return _borderVoxel;
}

/**
 * @param[in] voxel The value to use for voxels outside the volume.
 */
void RawVolume::setBorderValue(const Voxel& voxel) {
	_borderVoxel = voxel;
}

/**
 * @param x the @c x position of the voxel
 * @param y the @c y position of the voxel
 * @param z the @c z position of the voxel
 * @param voxel the value to which the voxel will be set
 * @return @c true if the voxel was placed, @c false if it was already the same voxel
 */
bool RawVolume::setVoxel(int32_t x, int32_t y, int32_t z, const Voxel& voxel) {
	return setVoxel(glm::ivec3(x, y, z), voxel);
}

/**
 * @param pos the 3D position of the voxel
 * @param voxel the value to which the voxel will be set
 * @return @c true if the voxel was placed, @c false if it was already the same voxel
 */
bool RawVolume::setVoxel(const glm::ivec3& pos, const Voxel& voxel) {
	const bool inside = _region.containsPoint(pos);
	core_assert_msg(inside, "Position is outside valid region %i:%i:%i (mins[%i:%i:%i], maxs[%i:%i:%i])",
			pos.x, pos.y, pos.z, _region.getLowerX(), _region.getLowerY(), _region.getLowerZ(),
			_region.getUpperX(), _region.getUpperY(), _region.getUpperZ());
	if (!inside) {
		return false;
	}
	const glm::ivec3& lowerCorner = _region.getLowerCorner();
	const int32_t localXPos = pos.x - lowerCorner.x;
	const int32_t localYPos = pos.y - lowerCorner.y;
	const int32_t iLocalZPos = pos.z - lowerCorner.z;
	const int index = localXPos + localYPos * width() + iLocalZPos * width() * height();
	if (_data[index].isSame(voxel)) {
		return false;
	}
	_data[index] = voxel;
	return true;
}

/**
 * This function should probably be made internal...
 */
void RawVolume::initialise(const Region& regValidRegion) {
	_region = regValidRegion;

	core_assert_msg(width() > 0, "Volume width must be greater than zero.");
	core_assert_msg(height() > 0, "Volume height must be greater than zero.");
	core_assert_msg(depth() > 0, "Volume depth must be greater than zero.");

	//Create the data
	const size_t size = width() * height() * depth() * sizeof(Voxel);
	_data = (Voxel*)core_malloc(size);
	core_assert_msg_always(_data != nullptr, "Failed to allocate the memory for a volume with the dimensions %i:%i:%i", width(),  height(), depth());

	// Clear to zeros
	clear();
}

void RawVolume::clear() {
	const size_t size = width() * height() * depth() * sizeof(Voxel);
	core_memset(_data, 0, size);
}

RawVolume::Sampler::Sampler(const RawVolume* volume) :
		_volume(const_cast<RawVolume*>(volume)), _region(volume->region()) {
}

RawVolume::Sampler::Sampler(const RawVolume& volume) :
		_volume(const_cast<RawVolume*>(&volume)), _region(volume.region()) {
}

RawVolume::Sampler::~Sampler() {
}

bool RawVolume::Sampler::setVoxel(const Voxel& voxel) {
	if (_currentPositionInvalid) {
		return false;
	}
	*_currentVoxel = voxel;
	return true;
}

bool RawVolume::Sampler::setPosition(int32_t xPos, int32_t yPos, int32_t zPos) {
	_posInVolume.x = xPos;
	_posInVolume.y = yPos;
	_posInVolume.z = zPos;

	const voxel::Region& region = this->region();
	_currentPositionInvalid = 0u;
	if (!region.containsPointInX(xPos)) {
		_currentPositionInvalid |= SAMPLER_INVALIDX;
	}
	if (!region.containsPointInY(yPos)) {
		_currentPositionInvalid |= SAMPLER_INVALIDY;
	}
	if (!region.containsPointInZ(zPos)) {
		_currentPositionInvalid |= SAMPLER_INVALIDZ;
	}

	// Then we update the voxel pointer
	if (currentPositionValid()) {
		const glm::ivec3& v3dLowerCorner = region.getLowerCorner();
		const int32_t iLocalXPos = xPos - v3dLowerCorner.x;
		const int32_t iLocalYPos = yPos - v3dLowerCorner.y;
		const int32_t iLocalZPos = zPos - v3dLowerCorner.z;
		const int32_t uVoxelIndex = iLocalXPos + iLocalYPos * _volume->width() + iLocalZPos * _volume->width() * _volume->height();

		_currentVoxel = _volume->_data + uVoxelIndex;
		return true;
	}
	_currentVoxel = nullptr;
	return false;
}

void RawVolume::Sampler::movePositive(math::Axis axis, uint32_t offset) {
	switch (axis) {
	case math::Axis::X:
		movePositiveX(offset);
		break;
	case math::Axis::Y:
		movePositiveY(offset);
		break;
	case math::Axis::Z:
		movePositiveZ(offset);
		break;
	default:
		break;
	}
}

void RawVolume::Sampler::movePositiveX(uint32_t offset) {
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.x += (int)offset;

	if (!region().containsPointInX(_posInVolume.x)) {
		_currentPositionInvalid |= SAMPLER_INVALIDX;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDX;
	}

	// Then we update the voxel pointer
	if (!bIsOldPositionValid) {
		setPosition(_posInVolume);
	} else if (currentPositionValid()) {
		_currentVoxel += (intptr_t)offset;
	} else {
		_currentVoxel = nullptr;
	}
}

void RawVolume::Sampler::movePositiveY(uint32_t offset) {
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.y += (int)offset;

	if (!region().containsPointInY(_posInVolume.y)) {
		_currentPositionInvalid |= SAMPLER_INVALIDY;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDY;
	}

	// Then we update the voxel pointer
	if (!bIsOldPositionValid) {
		setPosition(_posInVolume);
	} else if (currentPositionValid()) {
		_currentVoxel += (intptr_t)(_volume->width() * offset);
	} else {
		_currentVoxel = nullptr;
	}
}

void RawVolume::Sampler::movePositiveZ(uint32_t offset) {
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.z += (int)offset;

	if (!region().containsPointInZ(_posInVolume.z)) {
		_currentPositionInvalid |= SAMPLER_INVALIDZ;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDZ;
	}

	// Then we update the voxel pointer
	if (!bIsOldPositionValid) {
		setPosition(_posInVolume);
	} else if (currentPositionValid()) {
		_currentVoxel += (intptr_t)(_volume->width() * _volume->height() * offset);
	} else {
		_currentVoxel = nullptr;
	}
}

void RawVolume::Sampler::moveNegative(math::Axis axis, uint32_t offset) {
	switch (axis) {
	case math::Axis::X:
		moveNegativeX(offset);
		break;
	case math::Axis::Y:
		moveNegativeY(offset);
		break;
	case math::Axis::Z:
		moveNegativeZ(offset);
		break;
	default:
		break;
	}
}

void RawVolume::Sampler::moveNegativeX(uint32_t offset) {
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.x -= (int)offset;

	if (!region().containsPointInX(_posInVolume.x)) {
		_currentPositionInvalid |= SAMPLER_INVALIDX;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDX;
	}

	// Then we update the voxel pointer
	if (!bIsOldPositionValid) {
		setPosition(_posInVolume);
	} else if (currentPositionValid()) {
		_currentVoxel -= (intptr_t)offset;
	} else {
		_currentVoxel = nullptr;
	}
}

void RawVolume::Sampler::moveNegativeY(uint32_t offset) {
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.y -= (int)offset;

	if (!region().containsPointInY(_posInVolume.y)) {
		_currentPositionInvalid |= SAMPLER_INVALIDY;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDY;
	}

	// Then we update the voxel pointer
	if (!bIsOldPositionValid) {
		setPosition(_posInVolume);
	} else if (currentPositionValid()) {
		_currentVoxel -= (intptr_t)(_volume->width() * offset);
	} else {
		_currentVoxel = nullptr;
	}
}

void RawVolume::Sampler::moveNegativeZ(uint32_t offset) {
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.z -= (int)offset;

	if (!region().containsPointInZ(_posInVolume.z)) {
		_currentPositionInvalid |= SAMPLER_INVALIDZ;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDZ;
	}

	// Then we update the voxel pointer
	if (!bIsOldPositionValid) {
		setPosition(_posInVolume);
	} else if (currentPositionValid()) {
		_currentVoxel -= (intptr_t)(_volume->width() * _volume->height() * offset);
	} else {
		_currentVoxel = nullptr;
	}
}

}
