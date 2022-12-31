/**
 * @file
 * @defgroup Formats
 * @{
 * File formats.
 * @}
 */

#pragma once

#include "io/Stream.h"
#include "voxel/RawVolume.h"
#include "image/Image.h"
#include "voxelformat/FormatThumbnail.h"
#include <glm/fwd.hpp>

namespace voxel {
class Mesh;
class Palette;
}

namespace voxelformat {

class SceneGraph;
class SceneGraphNode;

// the max amount of voxels - [0-255]
static constexpr int MaxRegionSize = 256;

/**
 * @brief Base class for all voxel formats.
 *
 * @ingroup Formats
 */
class Format {
protected:
	/**
	 * @brief If you have to split the volumes in the scene graph because the format only supports a certain size, you
	 * can return the max size here. If the returned value is not a valid volume size (<= 0) the value is ignored.
	 * @sa singleVolume()
	 * @note @c singleVolume() and @c maxSize() don't work well together as the first would merge everything, and the latter
	 * would split it again if the max size was exceeded.
	 */
	virtual glm::ivec3 maxSize() const;

	/**
	 * @brief If a format only supports a single volume. If this returns true, the @¢ save() method gets a scene graph with
	 * only one model
	 * @sa maxSize()
	 * @note @c singleVolume() and @c maxSize() don't work well together as the first would merge everything, and the latter
	 * would split it again if the max size was exceeded.
	 */
	virtual bool singleVolume() const;

	/**
	 * @brief Checks whether the given chunk is empty (only contains air).
	 *
	 * @param v The volume
	 * @param maxSize The chunk size
	 * @param x The chunk position
	 * @param y The chunk position
	 * @param z The chunk position
	 */
	bool isEmptyBlock(const voxel::RawVolume *v, const glm::ivec3 &maxSize, int x, int y, int z) const;
	/**
	 * @brief Calculate the boundaries while aligning them to the given @c maxSize. This ensures that the
	 * calculated extends are exactly @c maxSize when iterating over them (and align relative to 0,0,0 and
	 * @c maxSize).
	 *
	 * @param[in] region The region to calculate the aligned mins/maxs for
	 * @param[in] maxSize The size of a single chunk to align with.
	 * @param[out] mins The extends of the aabb aligned with @c maxSize
	 * @param[out] maxs The extends of the aabb aligned with @c maxSize
	 */
	void calcMinsMaxs(const voxel::Region& region, const glm::ivec3 &maxSize, glm::ivec3 &mins, glm::ivec3 &maxs) const;
	/**
	 * @brief Split volumes according to their max size into several smaller volumes
	 * Some formats only support small volumes sizes per object - but multiple objects.
	 */
	void splitVolumes(const SceneGraph& srcSceneGraph, SceneGraph& destSceneGraph, const glm::ivec3 &maxSize, bool crop = false);

	/**
	 * Some formats are running loop that the user might want to interrupt with CTRL+c or the like. Long lasting loops should query
	 * this boolean and respect the users wish to quit the application.
	 */
	static bool stopExecution();

	static core::String stringProperty(const SceneGraphNode* node, const core::String &name, const core::String &defaultVal = "");
	static bool boolProperty(const SceneGraphNode* node, const core::String &name, bool defaultVal = false);
	static float floatProperty(const SceneGraphNode* node, const core::String &name, float defaultVal = 0.0f);
	static image::ImagePtr createThumbnail(const SceneGraph& sceneGraph, ThumbnailCreator thumbnailCreator, const ThumbnailContext &ctx);
	/**
	 * @param[in] sceneGraph The @c SceneGraph instance to save
	 * @param[in] filename The target file name. Some formats needs this next to the stream to identify or load additional files.
	 * @param[out] stream The target stream to write into
	 * @param[in] thumbnailCreator A callback that is either null or returns an instance of @c image::ImagePtr for the thumbnail of the
	 * given scene graph. Some formats have embedded screenshots.
	 */
	virtual bool saveGroups(const SceneGraph& sceneGraph, const core::String &filename, io::SeekableWriteStream& stream, ThumbnailCreator thumbnailCreator) = 0;
	/**
	 * @brief If the format supports multiple layers or groups, this method will give them to you as single volumes
	 */
	virtual bool loadGroups(const core::String &filename, io::SeekableReadStream& stream, SceneGraph& sceneGraph) = 0;
public:
	virtual ~Format() = default;

	/**
	 * Some formats have embedded screenshots of the model. This method doesn't load anything else than that image.
	 * @note Not supported by many formats.
	 */
	virtual image::ImagePtr loadScreenshot(const core::String &filename, io::SeekableReadStream& stream);

	/**
	 * @brief Only load the palette that is included in the format
	 * @note Not all voxel formats have a palette included - if they do and don't have this method implemented, they
	 * will go the expensive route. They will load all the nodes, all the voxels and just use the palette data. This
	 * means a lot of computation time is wasted and we should consider implementing this for as many as possible
	 * formats.
	 *
	 * @return the amount of colors found in the palette
	 */
	virtual size_t loadPalette(const core::String &filename, io::SeekableReadStream& stream, voxel::Palette &palette);
	virtual bool load(const core::String &filename, io::SeekableReadStream& stream, SceneGraph& sceneGraph);
	virtual bool save(const SceneGraph& sceneGraph, const core::String &filename, io::SeekableWriteStream& stream, ThumbnailCreator thumbnailCreator);
};

/**
 * @brief A format with only voxels - but no color attached
 *
 * @ingroup Formats
 */
class NoColorFormat : public Format {
};

/**
 * @brief A format with an embedded palette
 *
 * @ingroup Formats
 */
class PaletteFormat : public Format {
protected:
	virtual bool loadGroupsPalette(const core::String &filename, io::SeekableReadStream& stream, SceneGraph& sceneGraph, voxel::Palette &palette) = 0;

	/**
	 * @brief This indicates whether the format only supports one palette for the whole scene graph
	 */
	virtual bool onlyOnePalette() { return true; }
	bool loadGroups(const core::String &filename, io::SeekableReadStream& stream, SceneGraph& sceneGraph) override final;

public:
	size_t loadPalette(const core::String &filename, io::SeekableReadStream& stream, voxel::Palette &palette) override;
	bool save(const SceneGraph& sceneGraph, const core::String &filename, io::SeekableWriteStream& stream, ThumbnailCreator thumbnailCreator) override final;
};

/**
 * @brief A format that stores the voxels with rgba colors
 * @note These color are converted to an palette.
 *
 * @ingroup Formats
 */
class RGBAFormat : public Format {
protected:
	uint8_t _flattenFactor;
	virtual bool loadGroupsRGBA(const core::String &filename, io::SeekableReadStream &stream, SceneGraph &sceneGraph,
								const voxel::Palette &palette) {
		return false;
	}
	bool loadGroups(const core::String &filename, io::SeekableReadStream &stream, SceneGraph &sceneGraph) override;
	core::RGBA flattenRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) const;
	core::RGBA flattenRGB(core::RGBA rgba) const;
public:
	RGBAFormat();
};

/**
 * @ingroup Formats
 */
class RGBASinglePaletteFormat : public RGBAFormat {
};

}
