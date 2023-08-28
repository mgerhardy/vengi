/**
 * @file
 */

#pragma once

#include "Brush.h"
#include "core/ScopedPtr.h"
#include "voxel/Palette.h"
#include "voxel/RawVolume.h"

namespace voxedit {

class StampBrush : public Brush {
private:
	using Super = Brush;

protected:
	core::ScopedPtr<voxel::RawVolume> _volume;
	voxel::Palette _palette;
	glm::ivec3 _lastCursorPosition{0};
	bool _center = true;
	bool _continuous = false;

public:
	static constexpr int MaxSize = 32;

	void construct() override;
	bool execute(scenegraph::SceneGraph &sceneGraph, ModifierVolumeWrapper &wrapper,
				 const BrushContext &context) override;
	void setVolume(const voxel::RawVolume &volume, const voxel::Palette &palette);
	voxel::RawVolume *volume() const;
	bool active() const override;
	void reset() override;
	void update(const BrushContext &ctx, double nowSeconds) override;

	voxel::Region calcRegion(const BrushContext &context) const;

	void setCenterMode(bool center);
	bool centerMode() const;

	void setContinuousMode(bool single);
	bool continuousMode() const;

	/**
	 * @brief Either convert all voxels of the stamp to the given voxel, or create a new stamp with the given voxel
	 */
	void setVoxel(const voxel::Voxel &voxel, const voxel::Palette &palette);
	void convertToPalette(const voxel::Palette &palette);

	void setSize(const glm::ivec3 &size);

	bool load(const core::String &filename);
};

inline bool StampBrush::centerMode() const {
	return _center;
}

inline void StampBrush::setCenterMode(bool center) {
	_center = center;
}

inline bool StampBrush::continuousMode() const {
	return _continuous;
}

inline void StampBrush::setContinuousMode(bool continuous) {
	_continuous = continuous;
}

inline voxel::RawVolume *StampBrush::volume() const {
	return _volume;
}

inline bool StampBrush::active() const {
	return _volume != nullptr;
}

} // namespace voxedit
