/**
 * @file
 */

#pragma once

#include "core/Common.h"
#include "core/Trace.h"
#include "voxel/Face.h"
#include "voxel/Region.h"

namespace voxelutil {

enum class VisitorOrder {
	XYZ,
	ZYX,
	ZXY,
	XmZY,
	mXZY,
	mXmZY,
	mXZmY,
	XmZmY,
	mXmZmY,
	XZY,
	XZmY,
	YXZ,
	YZX,
	mYZX,
	YZmX,
	Max
};

/**
 * @brief Will skip air voxels on volume
 * @note Visitor condition
 */
struct SkipEmpty {
	inline bool operator()(const voxel::Voxel &voxel) const {
		return !isAir(voxel.getMaterial());
	}
};

/**
 * @note Visitor condition
 */
struct VisitAll {
	inline bool operator()(const voxel::Voxel &voxel) const {
		return true;
	}
};

/**
 * @note Visitor condition
 */
struct VisitColor {
	const uint8_t _color;
	VisitColor(uint8_t color) : _color(color) {
	}
	inline bool operator()(const voxel::Voxel &voxel) const {
		return voxel.getColor() == _color;
	}
};

/**
 * @note Visitor
 */
struct EmptyVisitor {
	inline void operator()(int x, int y, int z, const voxel::Voxel &voxel) const {
	}
};

template <class Volume, class Visitor, typename Condition = SkipEmpty>
int visitVolume(const Volume &volume, const voxel::Region &region, int xOff, int yOff, int zOff, Visitor &&visitor,
				Condition condition = Condition(), VisitorOrder order = VisitorOrder::ZYX) {
	core_trace_scoped(VisitVolume);
	int cnt = 0;

	typename Volume::Sampler sampler(volume);

	switch (order) {
	case VisitorOrder::XYZ:
		sampler.setPosition(region.getLowerCorner());
		for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveZ(zOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.movePositiveY(yOff);
			}
			sampler.movePositiveX(xOff);
		}
		break;
	case VisitorOrder::ZYX:
		sampler.setPosition(region.getLowerCorner());
		for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveX(xOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.movePositiveY(yOff);
			}
			sampler.movePositiveZ(zOff);
		}
		break;
	case VisitorOrder::ZXY:
		sampler.setPosition(region.getLowerCorner());
		for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveY(yOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.movePositiveX(xOff);
			}
			sampler.movePositiveZ(zOff);
		}
		break;
	case VisitorOrder::XZY:
		sampler.setPosition(region.getLowerCorner());
		for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveY(yOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.movePositiveZ(zOff);
			}
			sampler.movePositiveX(xOff);
		}
		break;
	case VisitorOrder::XZmY:
		sampler.setPosition(region.getLowerX(), region.getUpperY(), region.getLowerZ());
		for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getUpperY(); y >= region.getLowerY(); y -= yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.moveNegativeY(yOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.movePositiveZ(zOff);
			}
			sampler.movePositiveX(xOff);
		}
		break;
	case VisitorOrder::mXmZY:
		sampler.setPosition(region.getUpperX(), region.getLowerY(), region.getUpperZ());
		for (int32_t x = region.getUpperX(); x >= region.getLowerX(); x -= xOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getUpperZ(); z >= region.getLowerZ(); z -= zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveY(yOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.moveNegativeZ(zOff);
			}
			sampler.moveNegativeX(xOff);
		}
		break;
	case VisitorOrder::mXZY:
		sampler.setPosition(region.getUpperX(), region.getLowerY(), region.getLowerZ());
		for (int32_t x = region.getUpperX(); x >= region.getLowerX(); x -= xOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveY(yOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.movePositiveZ(zOff);
			}
			sampler.moveNegativeX(xOff);
		}
		break;
	case VisitorOrder::XmZY:
		sampler.setPosition(region.getLowerX(), region.getLowerY(), region.getUpperZ());
		for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getUpperZ(); z >= region.getLowerZ(); z -= zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveY(yOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.moveNegativeZ(zOff);
			}
			sampler.movePositiveX(xOff);
		}
		break;
	case VisitorOrder::XmZmY:
		sampler.setPosition(region.getLowerX(), region.getUpperY(), region.getUpperZ());
		for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getUpperZ(); z >= region.getLowerZ(); z -= zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getUpperY(); y >= region.getLowerY(); y -= yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.moveNegativeY(yOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.moveNegativeZ(zOff);
			}
			sampler.movePositiveX(xOff);
		}
		break;
	case VisitorOrder::mXmZmY:
		sampler.setPosition(region.getUpperX(), region.getUpperY(), region.getUpperZ());
		for (int32_t x = region.getUpperX(); x >= region.getLowerX(); x -= xOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getUpperZ(); z >= region.getLowerZ(); z -= zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getUpperY(); y >= region.getLowerY(); y -= yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.moveNegativeY(yOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.moveNegativeZ(zOff);
			}
			sampler.moveNegativeX(xOff);
		}
		break;
	case VisitorOrder::mXZmY:
		sampler.setPosition(region.getUpperX(), region.getUpperY(), region.getLowerZ());
		for (int32_t x = region.getUpperX(); x >= region.getLowerX(); x -= xOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getUpperY(); y >= region.getLowerY(); y -= yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.moveNegativeY(yOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.movePositiveZ(zOff);
			}
			sampler.moveNegativeX(xOff);
		}
		break;
	case VisitorOrder::YXZ:
		sampler.setPosition(region.getLowerCorner());
		for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveZ(zOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.movePositiveX(xOff);
			}
			sampler.movePositiveY(yOff);
		}
		break;
	case VisitorOrder::YZX:
		sampler.setPosition(region.getLowerCorner());
		for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveX(xOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.movePositiveZ(zOff);
			}
			sampler.movePositiveY(yOff);
		}
		break;
	case VisitorOrder::YZmX:
		sampler.setPosition(region.getUpperX(), region.getLowerY(), region.getLowerZ());
		for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t x = region.getUpperX(); x >= region.getLowerX(); x -= xOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.moveNegativeX(xOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.movePositiveZ(zOff);
			}
			sampler.movePositiveY(yOff);
		}
		break;
	case VisitorOrder::mYZX:
		sampler.setPosition(region.getLowerX(), region.getUpperY(), region.getLowerZ());
		for (int32_t y = region.getUpperY(); y >= region.getLowerY(); y -= yOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveX(xOff);
					if (!condition(voxel)) {
						continue;
					}
					visitor(x, y, z, voxel);
					++cnt;
				}
				sampler2.movePositiveZ(zOff);
			}
			sampler.moveNegativeY(yOff);
		}
		break;
	case VisitorOrder::Max:
		break;
	}

#undef LOOP
	return cnt;
}

template <class Volume, class Visitor, typename Condition = SkipEmpty>
int visitVolume(const Volume &volume, int xOff, int yOff, int zOff, Visitor &&visitor, Condition condition = Condition(), VisitorOrder order = VisitorOrder::ZYX) {
	const voxel::Region &region = volume.region();
	return visitVolume(volume, region, xOff, yOff, zOff, visitor, condition, order);
}

template <class Volume, class Visitor, typename Condition = SkipEmpty>
int visitVolume(const Volume &volume, Visitor &&visitor, Condition condition = Condition(), VisitorOrder order = VisitorOrder::ZYX) {
	return visitVolume(volume, 1, 1, 1, visitor, condition, order);
}

template <class Volume, class Visitor, typename Condition = SkipEmpty>
int visitVolume(const Volume &volume, const voxel::Region &region, Visitor &&visitor, Condition condition = Condition(), VisitorOrder order = VisitorOrder::ZYX) {
	return visitVolume(volume, region, 1, 1, 1, visitor, condition, order);
}

template <class Volume, class Visitor>
int visitSurfaceVolume(const Volume &volume, Visitor &&visitor, VisitorOrder order = VisitorOrder::ZYX) {
	int cnt = 0;
	const auto hullVisitor = [&cnt, &volume, visitor](int x, int y, int z, const voxel::Voxel &voxel) {
		if (visibleFaces(volume, x, y, z) != voxel::FaceBits::None) {
			visitor(x, y, z, voxel);
			++cnt;
		}
	};
	visitVolume(volume, hullVisitor, SkipEmpty(), order);
	return cnt;
}

/**
 * Visits the specified face of a volume and invokes the provided visitor function for each voxel on that face.
 * @note This is taking the surface voxels of the given face, that means that we are visiting the voxels that are
 *       visible from the outside when looking at the volume from the given face. If you don't want this behavior
 *       and want to visit all voxels on the face, set @c searchSurface to @c false
 *
 * @return The number of voxels visited.
 */
template<class Volume, class Visitor>
int visitFace(const Volume &volume, voxel::FaceNames face, Visitor &&visitor, bool searchSurface = false) {
	const voxel::Region &region = volume.region();
	const glm::ivec3 mins = region.getLowerCorner();
	const glm::ivec3 maxs = region.getUpperCorner();
	int cnt = 0;
	switch (face) {
	case voxel::FaceNames::Front:
		for (int x = mins.x; x <= maxs.x; ++x) {
			for (int y = maxs.y; y >= mins.y; --y) {
				for (int z = mins.z; z <= maxs.z; ++z) {
					if (!searchSurface || (visibleFaces(volume, x, y, z) & voxel::FaceBits::Front) != voxel::FaceBits::None) {
						visitor(x, y, z, volume.voxel(x, y, z));
						++cnt;
						break;
					}
				}
			}
		}
		break;
	case voxel::FaceNames::Back:
		for (int y = maxs.y; y >= mins.y; --y) {
			for (int x = mins.x; x <= maxs.x; ++x) {
				for (int z = maxs.z; z >= mins.z; --z) {
					if (!searchSurface || (visibleFaces(volume, x, y, z) & voxel::FaceBits::Back) != voxel::FaceBits::None) {
						visitor(x, y, z, volume.voxel(x, y, z));
						++cnt;
						break;
					}
				}
			}
		}
		break;
	case voxel::FaceNames::Left:
		for (int y = maxs.y; y >= mins.y; --y) {
			for (int z = mins.z; z <= maxs.z; ++z) {
				for (int x = mins.x; x <= maxs.x; ++x) {
					if (!searchSurface || (visibleFaces(volume, x, y, z) & voxel::FaceBits::Left) != voxel::FaceBits::None) {
						visitor(x, y, z, volume.voxel(x, y, z));
						++cnt;
						break;
					}
				}
			}
		}
		break;
	case voxel::FaceNames::Right:
		for (int y = maxs.y; y >= mins.y; --y) {
			for (int z = mins.z; z <= maxs.z; ++z) {
				for (int x = maxs.x; x >= mins.x; --x) {
					if (!searchSurface || (visibleFaces(volume, x, y, z) & voxel::FaceBits::Right) != voxel::FaceBits::None) {
						visitor(x, y, z, volume.voxel(x, y, z));
						++cnt;
						break;
					}
				}
			}
		}
		break;
	case voxel::FaceNames::Up:
		for (int z = maxs.z; z >= mins.z; --z) {
			for (int x = mins.x; x <= maxs.x; ++x) {
				for (int y = maxs.y; y >= mins.y; --y) {
					if (!searchSurface || (visibleFaces(volume, x, y, z) & voxel::FaceBits::Up) != voxel::FaceBits::None) {
						visitor(x, y, z, volume.voxel(x, y, z));
						++cnt;
						break;
					}
				}
			}
		}
		break;
	case voxel::FaceNames::Down:
		for (int x = mins.x; x <= maxs.x; ++x) {
			for (int z = maxs.z; z >= mins.z; --z) {
				for (int y = mins.y; y <= maxs.y; ++y) {
					if (!searchSurface || (visibleFaces(volume, x, y, z) & voxel::FaceBits::Down) != voxel::FaceBits::None) {
						visitor(x, y, z, volume.voxel(x, y, z));
						++cnt;
						break;
					}
				}
			}
		}
		break;
	default:
		break;
	}
	return cnt;
}

template<class Volume, class Visitor>
int visitUndergroundVolume(const Volume &volume, Visitor &&visitor, VisitorOrder order = VisitorOrder::ZYX) {
	int cnt = 0;
	const auto hullVisitor = [&cnt, &volume, visitor](int x, int y, int z, const voxel::Voxel &voxel) {
		if (visibleFaces(volume, x, y, z) == voxel::FaceBits::None) {
			visitor(x, y, z, voxel);
			++cnt;
		}
	};
	visitVolume(volume, hullVisitor, SkipEmpty(), order);
	return cnt;
}

} // namespace voxelutil
