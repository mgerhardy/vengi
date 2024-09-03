/**
 * @file
 */

#pragma once

#include "core/Common.h"
#include "core/Trace.h"
#include "math/Axis.h"
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
	mYmXZ,
	mYXmZ,
	mYmZmX,
	mZmXmY,
	ZmXY,
	YXmZ,
	ZXmY,
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
struct VisitEmpty {
	inline bool operator()(const voxel::Voxel &voxel) const {
		return isAir(voxel.getMaterial());
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

// the visitor can return a boolean value to indicate whether the inner loop should break
#define VISITOR_INNER_PART                                                                                             \
	if (!condition(voxel)) {                                                                                           \
		continue;                                                                                                      \
	}                                                                                                                  \
	if constexpr (std::is_same_v<decltype(visitor(x, y, z, voxel)), bool>) {                                          \
		const bool breakInnerLoop = visitor(x, y, z, voxel);                                                           \
		++cnt;                                                                                                         \
		if (breakInnerLoop) {                                                                                          \
			break;                                                                                                     \
		}                                                                                                              \
	} else {                                                                                                           \
		visitor(x, y, z, voxel);                                                                                       \
		++cnt;                                                                                                         \
	}

template<class Volume, class Visitor, typename Condition = SkipEmpty>
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
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
					VISITOR_INNER_PART
				}
				sampler2.movePositiveZ(zOff);
			}
			sampler.moveNegativeY(yOff);
		}
		break;
	case VisitorOrder::mYmXZ:
		sampler.setPosition(region.getUpperX(), region.getUpperY(), region.getLowerZ());
		for (int32_t y = region.getUpperY(); y >= region.getLowerY(); y -= yOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t x = region.getUpperX(); x >= region.getLowerX(); x -= xOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveZ(zOff);
					VISITOR_INNER_PART
				}
				sampler2.moveNegativeX(xOff);
			}
			sampler.moveNegativeY(yOff);
		}
		break;
	case VisitorOrder::mYXmZ:
		sampler.setPosition(region.getLowerX(), region.getUpperY(), region.getUpperZ());
		for (int32_t y = region.getUpperY(); y >= region.getLowerY(); y -= yOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t z = region.getUpperZ(); z >= region.getLowerZ(); z -= zOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.moveNegativeZ(zOff);
					VISITOR_INNER_PART
				}
				sampler2.movePositiveX(xOff);
			}
			sampler.moveNegativeY(yOff);
		}
		break;
	case VisitorOrder::mYmZmX:
		sampler.setPosition(region.getUpperX(), region.getUpperY(), region.getUpperZ());
		for (int32_t y = region.getUpperY(); y >= region.getLowerY(); y -= yOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t z = region.getUpperZ(); z >= region.getLowerZ(); z -= zOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t x = region.getUpperX(); x >= region.getLowerX(); x -= xOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.moveNegativeX(xOff);
					VISITOR_INNER_PART
				}
				sampler2.moveNegativeZ(zOff);
			}
			sampler.moveNegativeY(yOff);
		}
		break;
	case VisitorOrder::mZmXmY:
		sampler.setPosition(region.getUpperX(), region.getUpperY(), region.getUpperZ());
		for (int32_t z = region.getUpperZ(); z >= region.getLowerZ(); z -= zOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t x = region.getUpperX(); x >= region.getLowerX(); x -= xOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getUpperY(); y >= region.getLowerY(); y -= yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.moveNegativeY(yOff);
					VISITOR_INNER_PART
				}
				sampler2.moveNegativeX(xOff);
			}
			sampler.moveNegativeZ(zOff);
		}
		break;
	case VisitorOrder::ZmXY:
		sampler.setPosition(region.getUpperX(), region.getLowerY(), region.getLowerZ());
		for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t x = region.getUpperX(); x >= region.getLowerX(); x -= xOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.movePositiveY(yOff);
					VISITOR_INNER_PART
				}
				sampler2.moveNegativeX(xOff);
			}
			sampler.movePositiveZ(zOff);
		}
		break;
	case VisitorOrder::YXmZ:
		sampler.setPosition(region.getLowerX(), region.getLowerY(), region.getUpperZ());
		for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y += yOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t z = region.getUpperZ(); z >= region.getLowerZ(); z -= zOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.moveNegativeZ(zOff);
					VISITOR_INNER_PART
				}
				sampler2.movePositiveX(xOff);
			}
			sampler.movePositiveY(yOff);
		}
		break;
	case VisitorOrder::ZXmY:
		sampler.setPosition(region.getLowerX(), region.getUpperY(), region.getLowerZ());
		for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z += zOff) {
			typename Volume::Sampler sampler2 = sampler;
			for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x += xOff) {
				typename Volume::Sampler sampler3 = sampler2;
				for (int32_t y = region.getUpperY(); y >= region.getLowerY(); y -= yOff) {
					const voxel::Voxel &voxel = sampler3.voxel();
					sampler3.moveNegativeY(yOff);
					VISITOR_INNER_PART
				}
				sampler2.movePositiveX(xOff);
			}
			sampler.movePositiveZ(zOff);
		}
		break;
	case VisitorOrder::Max:
		break;
	}

#undef LOOP
	return cnt;
}

template<class Volume, class Visitor, typename Condition = SkipEmpty>
int visitVolume(const Volume &volume, int xOff, int yOff, int zOff, Visitor &&visitor,
				Condition condition = Condition(), VisitorOrder order = VisitorOrder::ZYX) {
	const voxel::Region &region = volume.region();
	return visitVolume(volume, region, xOff, yOff, zOff, visitor, condition, order);
}

template<class Volume, class Visitor, typename Condition = SkipEmpty>
int visitVolume(const Volume &volume, Visitor &&visitor, Condition condition = Condition(),
				VisitorOrder order = VisitorOrder::ZYX) {
	return visitVolume(volume, 1, 1, 1, visitor, condition, order);
}

template<class Volume, class Visitor, typename Condition = SkipEmpty>
int visitVolume(const Volume &volume, const voxel::Region &region, Visitor &&visitor, Condition condition = Condition(),
				VisitorOrder order = VisitorOrder::ZYX) {
	return visitVolume(volume, region, 1, 1, 1, visitor, condition, order);
}

template<class Volume, class Visitor>
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
 * @return The number of voxels visited.
 */
template<class Volume, class Visitor>
int visitFace(const Volume &volume, const voxel::Region &region, voxel::FaceNames face, Visitor &&visitor, VisitorOrder order,
			  bool searchSurface = false) {
	typename Volume::Sampler sampler(volume);
	voxel::FaceBits faceBits = voxel::faceBits(face);
	constexpr bool skipEmpty = true;

	auto visitorInternal = [&] (int x, int y, int z, const voxel::Voxel &voxel) {
		if (!searchSurface) {
			visitor(x, y, z, voxel);
			return true;
		}
		typename Volume::Sampler sampler2(volume);
		sampler2.setPosition(x, y, z);
		if ((visibleFaces(sampler2, skipEmpty) & faceBits) != voxel::FaceBits::None) {
			visitor(x, y, z, voxel);
			return true;
		}
		return false;
	};
	return visitVolume(volume, region, 1, 1, 1, visitorInternal, VisitAll(), order);
}

/**
 * @return The number of voxels visited.
 */
template<class Volume, class Visitor>
int visitFace(const Volume &volume, const voxel::Region &region, voxel::FaceNames face, Visitor &&visitor,
			  bool searchSurface = false) {
	// only the last axis matters here
	VisitorOrder visitorOrder;
	switch (face) {
	case voxel::FaceNames::Front:
		visitorOrder = VisitorOrder::mYmXZ;
		break;
	case voxel::FaceNames::Back:
		visitorOrder = VisitorOrder::mYXmZ;
		break;
	case voxel::FaceNames::Right:
		visitorOrder = VisitorOrder::mYmZmX;
		break;
	case voxel::FaceNames::Left:
		visitorOrder = VisitorOrder::mYZX;
		break;
	case voxel::FaceNames::Up:
		visitorOrder = VisitorOrder::mZmXmY;
		break;
	case voxel::FaceNames::Down:
		visitorOrder = VisitorOrder::ZmXY;
		break;
	default:
		return 0;
	}
	return visitFace(volume, region, face, visitor, visitorOrder, searchSurface);
}

template<class Volume, class Visitor>
int visitFace(const Volume &volume, voxel::FaceNames face, Visitor &&visitor, VisitorOrder order = VisitorOrder::Max, bool searchSurface = false) {
	const voxel::Region &region = volume.region();
	if (order == VisitorOrder::Max) {
		return visitFace(volume, region, face, visitor, searchSurface);
	}
	return visitFace(volume, region, face, visitor, order, searchSurface);
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
