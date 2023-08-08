/**
 * @file
 */

#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/fwd.hpp>
#include <glm/common.hpp>
#include "core/String.h"
#include <stdint.h>

namespace math {
class Random;
}

namespace voxel {

/**
 * Represents a part of a Volume.
 *
 * Many operations in PolyVox are constrained to only part of a volume. For example, when running the surface extractors
 * it is unlikely that you will want to run it on the whole volume at once, as this will give a very large mesh which may
 * be too much to render. Instead you will probably want to run a surface extractor a number of times on different parts
 * of the volume, there by giving a number of meshes which can be culled and rendered separately.
 *
 * The Region class is used to define these parts (regions) of the volume. Essentially it consists of an upper and lower
 * bound which specify the range of voxels positions considered to be part of the region. Note that these bounds are
 * <em>inclusive</em>.
 *
 * As well as the expected set of getters and setters, this class also provide utility functions for increasing and decreasing
 * the size of the Region, shifting the Region in 3D space, testing whether it contains a given position, enlarging it so that
 * it does contain a given position, cropping it to another Region, and various other utility functions.
 *
 * @note The dimensions of a region can be measured either in voxels or in cells. See the manual for more information
 * about these definitions.
 */
class Region {
public:
	Region();
	Region(const glm::ivec3& mins, const glm::ivec3& maxs);
	Region(int32_t minsx, int32_t minsy, int32_t minsz, int32_t maxsx, int32_t maxsy, int32_t maxsz);
	Region(int mins, int maxs);

	static const Region InvalidRegion;

	bool operator==(const Region& rhs) const;
	bool operator!=(const Region& rhs) const;

	/** Moves the Region by the amount specified. */
	Region& operator+=(const glm::ivec3& amount);

	/** Gets the 'x' position of the centre. */
	int32_t getCenterX() const;
	/** Gets the 'y' position of the centre. */
	int32_t getCenterY() const;
	/** Gets the 'z' position of the centre. */
	int32_t getCenterZ() const;
	/** Gets the 'x' position of the lower corner. */
	int32_t getLowerX() const;
	/** Gets the 'y' position of the lower corner. */
	int32_t getLowerY() const;
	/** Gets the 'z' position of the lower corner. */
	int32_t getLowerZ() const;
	/** Gets the 'x' position of the upper corner. */
	int32_t getUpperX() const;
	/** Gets the 'y' position of the upper corner. */
	int32_t getUpperY() const;
	/** Gets the 'z' position of the upper corner. */
	int32_t getUpperZ() const;

	/** Gets the centre of the region */
	const glm::ivec3& getCenter() const;
	glm::vec3 calcCenterf() const;
	/** normalized pivot */
	const glm::vec3& pivot() const;
	glm::vec3 worldPivot() const;
	void setPivot(const glm::vec3 &normalizedPivot);
	/** Gets the position of the lower corner. */
	const glm::ivec3& getLowerCorner() const;
	/** Gets the position of the upper corner. */
	const glm::ivec3& getUpperCorner() const;

	glm::vec3 getLowerCornerf() const;
	glm::vec3 getUpperCornerf() const;

	glm::ivec3 getRandomPosition(math::Random& random) const;

	/** Gets the width of the region measured in voxels. */
	int32_t getWidthInVoxels() const;
	/** Gets the height of the region measured in voxels. */
	int32_t getHeightInVoxels() const;
	/** Gets the depth of the region measured in voxels. */
	int32_t getDepthInVoxels() const;
	/** Gets the dimensions of the region measured in voxels. */
	const glm::ivec3& getDimensionsInVoxels() const;

	/** Gets the width of the region measured in cells. */
	int32_t getWidthInCells() const;
	/** Gets the height of the region measured in cells. */
	int32_t getHeightInCells() const;
	/** Gets the depth of the region measured in cells. */
	int32_t getDepthInCells() const;
	/** Gets the dimensions of the region measured in cells. */
	const glm::ivec3& getDimensionsInCells() const;

	/** Sets the 'x' position of the lower corner. */
	void setLowerX(int32_t x);
	/** Sets the 'y' position of the lower corner. */
	void setLowerY(int32_t y0f128);
	/** Sets the 'z' position of the lower corner. */
	void setLowerZ(int32_t z);
	/** Sets the 'x' position of the upper corner. */
	void setUpperX(int32_t x);
	/** Sets the 'y' position of the upper corner. */
	void setUpperY(int32_t y);
	/** Sets the 'z' position of the upper corner. */
	void setUpperZ(int32_t z);

	glm::ivec3 moveInto(int32_t x, int32_t y, int32_t z) const;

	/** Sets the position of the lower corner. */
	void setLowerCorner(const glm::ivec3& mins);
	/** Sets the position of the upper corner. */
	void setUpperCorner(const glm::ivec3& maxs);

	/**
	 * @return true if the given point is exactly on the region border
	 */
	bool isOnBorder(const glm::ivec3& pos) const;
	bool isOnBorderX(const int x) const;
	bool isOnBorderY(const int y) const;
	bool isOnBorderZ(const int z) const;

	/** Tests whether the given point is contained in this Region. */
	bool containsPoint(float fX, float fY, float fZ) const;
	/** Tests whether the given point is contained in this Region. */
	bool containsPoint(const glm::vec3& pos) const;
	/** Tests whether the given point is contained in this Region. */
	bool containsPoint(int32_t iX, int32_t iY, int32_t iZ) const;
	/** Tests whether the given point is contained in this Region. */
	bool containsPoint(const glm::ivec3& pos) const;
	/** Tests whether the given position is contained in the 'x' range of this Region. */
	bool containsPointInX(float pos) const;
	/** Tests whether the given position is contained in the 'x' range of this Region. */
	bool containsPointInX(int32_t pos) const;
	/** Tests whether the given position is contained in the 'y' range of this Region. */
	bool containsPointInY(float pos) const;
	/** Tests whether the given position is contained in the 'y' range of this Region. */
	bool containsPointInY(int32_t pos) const;
	/** Tests whether the given position is contained in the 'z' range of this Region. */
	bool containsPointInZ(float pos) const;
	/** Tests whether the given position is contained in the 'z' range of this Region. */
	bool containsPointInZ(int32_t pos) const;

	/** Tests whether the given Region is contained in this Region. */
	bool containsRegion(const Region& reg) const;

	/** Enlarges the Region so that it contains the specified position. */
	void accumulate(int32_t iX, int32_t iY, int32_t iZ);
	/** Enlarges the Region so that it contains the specified position. */
	void accumulate(const glm::ivec3& v3dPos);

	/** Enlarges the Region so that it contains the specified Region. */
	void accumulate(const Region& reg);

	/** Crops the extents of this Region according to another Region. */
	void cropTo(const Region& other);

	/** Grows this region by the amount specified. */
	void grow(int32_t amount);
	/** Grows this region by the amounts specified. */
	void grow(int32_t amountX, int32_t amountY, int32_t amountZ);
	/** Grows this region by the amounts specified. */
	void grow(const glm::ivec3& v3dAmount);

	/** Tests whether all components of the upper corner are at least
	 * as great as the corresponding components of the lower corner. */
	bool isValid() const;

	/**
	 * @return The amount of possible voxels in this region.
	 */
	int voxels() const;
	int stride() const;

	/** Moves the Region by the amount specified. */
	void shift(int32_t amountX, int32_t amountY, int32_t amountZ);
	/** Moves the Region by the amount specified. */
	void shift(const glm::ivec3& v3dAmount);
	/** Moves the lower corner of the Region by the amount specified. */
	void shiftLowerCorner(int32_t x, int32_t y, int32_t z);
	/** Moves the lower corner of the Region by the amount specified. */
	void shiftLowerCorner(const glm::ivec3& v3dAmount);
	/** Moves the upper corner of the Region by the amount specified. */
	void shiftUpperCorner(int32_t x, int32_t y, int32_t z);
	/** Moves the upper corner of the Region by the amount specified. */
	void shiftUpperCorner(const glm::ivec3& v3dAmount);

	/** Shrinks this region by the amount specified. */
	void shrink(int32_t amount);
	/** Shrinks this region by the amounts specified. */
	void shrink(int32_t amountX, int32_t amountY, int32_t amountZ);
	/** Shrinks this region by the amounts specified. */
	void shrink(const glm::ivec3& v3dAmount);
	Region rotate(const glm::mat4 &mat, const glm::vec3 &pivot) const;

	core::String toString(bool center = false) const;

private:
	void update();

	alignas(16) glm::ivec3 _mins;
	alignas(16) glm::ivec3 _maxs;
	alignas(16) glm::ivec3 _width;
	alignas(16) glm::ivec3 _voxels;
	alignas(16) glm::ivec3 _center;
	alignas(16) glm::vec3 _pivot {0.0f};
	int _stride;
};

/**
 * @return The 'x' position of the centre.
 */
inline int32_t Region::getCenterX() const {
	return _center.x;
}

/**
 * @return The 'y' position of the centre.
 */
inline int32_t Region::getCenterY() const {
	return _center.y;
}

/**
 * @return The 'z' position of the centre.
 */
inline int32_t Region::getCenterZ() const {
	return _center.z;
}

/**
 * @return The 'x' position of the lower corner.
 */
inline int32_t Region::getLowerX() const {
	return _mins.x;
}

/**
 * @return The 'y' position of the lower corner.
 */
inline int32_t Region::getLowerY() const {
	return _mins.y;
}

/**
 * @return The 'z' position of the lower corner.
 */
inline int32_t Region::getLowerZ() const {
	return _mins.z;
}

/**
 * @return The 'x' position of the upper corner.
 */
inline int32_t Region::getUpperX() const {
	return _maxs.x;
}

/**
 * @return The 'y' position of the upper corner.
 */
inline int32_t Region::getUpperY() const {
	return _maxs.y;
}

/**
 * @return The 'z' position of the upper corner.
 */
inline int32_t Region::getUpperZ() const {
	return _maxs.z;
}

/**
 * @return The width of the region measured in voxels.
 * @sa getWidthInCells()
 */
inline int32_t Region::getWidthInVoxels() const {
	return _voxels.x;
}

/**
 * @return The height of the region measured in voxels.
 * @sa getHeightInCells()
 */
inline int32_t Region::getHeightInVoxels() const {
	return _voxels.y;
}

/**
 * @return The depth of the region measured in voxels.
 * @sa getDepthInCells()
 */
inline int32_t Region::getDepthInVoxels() const {
	return _voxels.z;
}

/**
 * @return The width of the region measured in cells.
 * @sa getWidthInVoxels()
 */
inline int32_t Region::getWidthInCells() const {
	return _width.x;
}

/**
 * @return The height of the region measured in cells.
 * @sa getHeightInVoxels()
 */
inline int32_t Region::getHeightInCells() const {
	return _width.y;
}

/**
 * @return The depth of the region measured in cells.
 * @sa getDepthInVoxels()
 */
inline int32_t Region::getDepthInCells() const {
	return _width.z;
}

inline int Region::stride() const {
	return _stride;
}

/**
 * @param x The new 'x' position of the lower corner.
 */
inline void Region::setLowerX(int32_t x) {
	_mins.x = x;
	update();
}

/**
 * @param y The new 'y' position of the lower corner.
 */
inline void Region::setLowerY(int32_t y) {
	_mins.y = y;
	update();
}

/**
 * @param z The new 'z' position of the lower corner.
 */
inline void Region::setLowerZ(int32_t z) {
	_mins.z = z;
	update();
}

/**
 * @param x The new 'x' position of the upper corner.
 */
inline void Region::setUpperX(int32_t x) {
	_maxs.x = x;
	update();
}

/**
 * @param y The new 'y' position of the upper corner.
 */
inline void Region::setUpperY(int32_t y) {
	_maxs.y = y;
	update();
}

/**
 * @param z The new 'z' position of the upper corner.
 */
inline void Region::setUpperZ(int32_t z) {
	_maxs.z = z;
	update();
}

inline Region::Region(int mins, int maxs) :
		Region(mins, mins, mins, maxs, maxs, maxs) {
}

/**
 * Constructs a Region and clears all extents to zero.
 */
inline Region::Region() :
		_mins(0), _maxs(0), _width(0), _voxels(1), _center(0), _stride(0) {
}

/**
 * Constructs a Region and sets the extents to the specified values.
 * @param minsx The desired lower 'x' extent of the Region.
 * @param minsy The desired lower 'y' extent of the Region.
 * @param minsz The desired lower 'z' extent of the Region.
 * @param maxsx The desired upper 'x' extent of the Region.
 * @param maxsy The desired upper 'y' extent of the Region.
 * @param maxsz The desired upper 'z' extent of the Region.
 */
inline Region::Region(int32_t minsx, int32_t minsy, int32_t minsz, int32_t maxsx, int32_t maxsy, int32_t maxsz) :
		_mins(minsx, minsy, minsz), _maxs(maxsx, maxsy, maxsz), _width(_maxs - _mins), _voxels(_width + 1), _center(_mins + _width / 2),
		_stride(_voxels.x * _voxels.y) {
}

/**
 * Two regions are considered equal if all their extents match.
 * @param rhs The Region to compare to.
 * @return true if the Regions match.
 * @sa operator!=
 */
inline bool Region::operator==(const Region& rhs) const {
	return ((_mins.x == rhs._mins.x) && (_mins.y == rhs._mins.y) && (_mins.z == rhs._mins.z) && (_maxs.x == rhs._maxs.x) && (_maxs.y == rhs._maxs.y)
			&& (_maxs.z == rhs._maxs.z));
}

/**
 * Two regions are considered different if any of their extents differ.
 * @param rhs The Region to compare to.
 * @return true if the Regions are different.
 * @sa operator==
 */
inline bool Region::operator!=(const Region& rhs) const {
	return !(*this == rhs);
}

inline bool Region::isOnBorderX(const int x) const {
	return x == _maxs.x || x == _mins.x;
}

inline bool Region::isOnBorderY(const int y) const {
	return y == _maxs.y || y == _mins.y;
}

inline bool Region::isOnBorderZ(const int z) const {
	return z == _maxs.z || z == _mins.z;
}

/**
 * The test is inclusive such
 * that positions lying exactly on the edge of the Region are considered to be inside it.
 * @param fX The 'x' position of the point to test.
 * @param fY The 'y' position of the point to test.
 * @param fZ The 'z' position of the point to test.
 */
inline bool Region::containsPoint(float fX, float fY, float fZ) const {
	return (fX <= _maxs.x) && (fY <= _maxs.y) && (fZ <= _maxs.z) && (fX >= _mins.x) && (fY >= _mins.y) &&
		   (fZ >= _mins.z);
}

/**
 * The test is inclusive such
 * that positions lying exactly on the edge of the Region are considered to be inside it.
 * @param iX The 'x' position of the point to test.
 * @param iY The 'y' position of the point to test.
 * @param iZ The 'z' position of the point to test.
 */
inline bool Region::containsPoint(int32_t iX, int32_t iY, int32_t iZ) const {
	return (iX <= _maxs.x) && (iY <= _maxs.y) && (iZ <= _maxs.z) && (iX >= _mins.x) && (iY >= _mins.y) &&
		   (iZ >= _mins.z);
}

/**
 * The test is inclusive such
 * that positions lying exactly on the edge of the Region are considered to be inside it.
 * @param pos The position to test.
 */
inline bool Region::containsPointInX(float pos) const {
	return (pos <= _maxs.x) && (pos >= _mins.x);
}

/**
 * The test is inclusive such
 * that positions lying exactly on the edge of the Region are considered to be inside it.
 * @param pos The position to test.
 */
inline bool Region::containsPointInX(int32_t pos) const {
	return (pos <= _maxs.x) && (pos >= _mins.x);
}

/**
 * The test is inclusive such
 * that positions lying exactly on the edge of the Region are considered to be inside it.
 * @param pos The position to test.
 */
inline bool Region::containsPointInY(float pos) const {
	return (pos <= _maxs.y) && (pos >= _mins.y);
}

/**
 * The test is inclusive such
 * that positions lying exactly on the edge of the Region are considered to be inside it.
 * @param pos The position to test.
 */
inline bool Region::containsPointInY(int32_t pos) const {
	return (pos <= _maxs.y) && (pos >= _mins.y);
}

/**
 * The test is inclusive such
 * that positions lying exactly on the edge of the Region are considered to be inside it.
 * @param pos The position to test.
 */
inline bool Region::containsPointInZ(float pos) const {
	return (pos <= _maxs.z) && (pos >= _mins.z);
}

/**
 * The test is inclusive such
 * that positions lying exactly on the edge of the Region are considered to be inside it.
 * @param pos The position to test.
 */
inline bool Region::containsPointInZ(int32_t pos) const {
	return (pos <= _maxs.z) && (pos >= _mins.z);
}

/**
 * The test is inclusive such
 * that a region is considered to be inside of itself.
 * @param reg The region to test.
 */
inline bool Region::containsRegion(const Region& reg) const {
	return (reg._maxs.x <= _maxs.x) && (reg._maxs.y <= _maxs.y) && (reg._maxs.z <= _maxs.z) && (reg._mins.x >= _mins.x)
			&& (reg._mins.y >= _mins.y) && (reg._mins.z >= _mins.z);
}

inline bool Region::isValid() const {
	return _maxs.x >= _mins.x && _maxs.y >= _mins.y && _maxs.z >= _mins.z;
}

/**
 * @param amountX The amount to move the Region by in 'x'.
 * @param amountY The amount to move the Region by in 'y'.
 * @param amountZ The amount to move the Region by in 'z'.
 */
inline void Region::shift(int32_t amountX, int32_t amountY, int32_t amountZ) {
	shiftLowerCorner(amountX, amountY, amountZ);
	shiftUpperCorner(amountX, amountY, amountZ);
}

/**
 * @param x The amount to move the lower corner by in 'x'.
 * @param y The amount to move the lower corner by in 'y'.
 * @param z The amount to move the lower corner by in 'z'.
 */
inline void Region::shiftLowerCorner(int32_t x, int32_t y, int32_t z) {
	_mins.x += x;
	_mins.y += y;
	_mins.z += z;
	update();
}

/**
 * @param x The amount to move the upper corner by in 'x'.
 * @param y The amount to move the upper corner by in 'y'.
 * @param z The amount to move the upper corner by in 'z'.
 */
inline void Region::shiftUpperCorner(int32_t x, int32_t y, int32_t z) {
	_maxs.x += x;
	_maxs.y += y;
	_maxs.z += z;
	update();
}

/**
 * The same amount of shrinkage is applied in all directions. Negative shrinkage
 * is possible but you should prefer the grow() function for clarity.
 * @param amount The amount to shrink by.
 */
inline void Region::shrink(int32_t amount) {
	_mins.x += amount;
	_mins.y += amount;
	_mins.z += amount;

	_maxs.x -= amount;
	_maxs.y -= amount;
	_maxs.z -= amount;
	update();
}

/**
 * The amount can be specified separately for each direction. Negative shrinkage
 * is possible but you should prefer the grow() function for clarity.
 * @param amountX The amount to shrink by in 'x'.
 * @param amountY The amount to shrink by in 'y'.
 * @param amountZ The amount to shrink by in 'z'.
 */
inline void Region::shrink(int32_t amountX, int32_t amountY, int32_t amountZ) {
	_mins.x += amountX;
	_mins.y += amountY;
	_mins.z += amountZ;

	_maxs.x -= amountX;
	_maxs.y -= amountY;
	_maxs.z -= amountZ;
	update();
}

inline bool intersects(const Region& a, const Region& b) {
	// No intersection if separated along an axis.
	if (a.getUpperX() < b.getLowerX() || a.getLowerX() > b.getUpperX())
		return false;
	if (a.getUpperY() < b.getLowerY() || a.getLowerY() > b.getUpperY())
		return false;
	if (a.getUpperZ() < b.getLowerZ() || a.getLowerZ() > b.getUpperZ())
		return false;

	// Overlapping on all axes means Regions are intersecting.
	return true;
}

extern void logRegion(const char *ctx, const voxel::Region& region);

}
