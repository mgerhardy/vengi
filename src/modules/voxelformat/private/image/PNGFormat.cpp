/**
 * @file
 */

#include "PNGFormat.h"
#include "core/GameConfig.h"
#include "core/Log.h"
#include "core/ScopedPtr.h"
#include "core/StringUtil.h"
#include "core/Var.h"
#include "image/Image.h"
#include "io/Archive.h"
#include "io/FilesystemEntry.h"
#include "io/Stream.h"
#include "palette/Palette.h"
#include "palette/PaletteLookup.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/SceneGraphNode.h"
#include "voxel/RawVolumeWrapper.h"
#include "voxel/Voxel.h"
#include "voxelutil/ImageUtils.h"

namespace voxelformat {

#define MaxHeightmapWidth 4096
#define MaxHeightmapHeight 4096

static int extractLayerFromFilename(const core::String &filename) {
	core::String name = core::string::extractFilename(filename);
	size_t sep = name.rfind('-');
	if (sep == core::String::npos) {
		Log::error("Invalid image name %s", name.c_str());
		return false;
	}
	const int layer = name.substr(sep + 1).toInt();
	return layer;
}

bool PNGFormat::importSlices(scenegraph::SceneGraph &sceneGraph, const palette::Palette &palette,
							 const io::ArchiveFiles &entities) const {
	const core::Path filename = entities.front().fullPath;
	Log::debug("Use %s as reference image", filename.c_str());
	image::ImagePtr referenceImage = image::loadImage(filename.str());
	if (!referenceImage || !referenceImage->isLoaded()) {
		Log::error("Failed to load first image as reference %s", filename.c_str());
		return false;
	}
	const int imageWidth = referenceImage->width();
	const int imageHeight = referenceImage->height();
	referenceImage.release();
	int minsZ = 1000000;
	int maxsZ = -1000000;

	for (const auto &entity : entities) {
		const core::Path &layerFilename = entity.fullPath;
		const int layer = extractLayerFromFilename(layerFilename.str());
		minsZ = glm::min(minsZ, layer);
		maxsZ = glm::max(maxsZ, layer);
	}

	voxel::Region region(0, 0, minsZ, imageWidth - 1, imageHeight - 1, maxsZ);
	voxel::RawVolume *volume = new voxel::RawVolume(region);
	scenegraph::SceneGraphNode node(scenegraph::SceneGraphNodeType::Model);
	node.setVolume(volume, true);
	node.setName(core::string::extractFilename(filename.str()));

	for (const auto &entity : entities) {
		const core::Path &layetFilename = entity.fullPath;
		const image::ImagePtr &image = image::loadImage(layetFilename.str());
		if (!image || !image->isLoaded()) {
			Log::error("Failed to load image %s", layetFilename.c_str());
			return false;
		}
		if (imageWidth != image->width() || imageHeight != image->height()) {
			Log::error("Image %s has different dimensions than the first image (%d:%d) vs (%d:%d)", layetFilename.c_str(),
					   image->width(), image->height(), imageWidth, imageHeight);
			return false;
		}
		const int layer = extractLayerFromFilename(layetFilename.str());
		Log::debug("Import layer %i of image %s", layer, layetFilename.c_str());
		for (int y = 0; y < imageHeight; ++y) {
			for (int x = 0; x < imageWidth; ++x) {
				const core::RGBA &color = flattenRGB(image->colorAt(x, y));
				if (color.a == 0) {
					continue;
				}
				const int palIdx = palette.getClosestMatch(color);
				volume->setVoxel(x, y, layer, voxel::createVoxel(palette, palIdx));
			}
		}
	}
	if (sceneGraph.emplace(core::move(node)) == InvalidNodeId) {
		Log::error("Failed to add node to scene graph");
		return false;
	}
	return true;
}

bool PNGFormat::importAsHeightmap(scenegraph::SceneGraph &sceneGraph, const palette::Palette &palette,
								  const core::String &filename, const io::ArchivePtr &archive) const {
	image::ImagePtr image = image::loadImage(filename);
	if (image->width() > MaxHeightmapWidth || image->height() >= MaxHeightmapHeight) {
		Log::warn("Skip creating heightmap - image dimensions exceeds the max allowed boundaries");
		return false;
	}
	const bool coloredHeightmap = image->depth() == 4 && !image->isGrayScale();
	const int maxHeight = voxelutil::importHeightMaxHeight(image, coloredHeightmap);
	if (maxHeight == 0) {
		Log::error("There is no height in either the red channel or the alpha channel");
		return false;
	}
	if (maxHeight == 1) {
		Log::warn("There is no height value in the image - it is imported as flat plane");
	}
	Log::info("Generate from heightmap (%i:%i) with max height of %i", image->width(), image->height(), maxHeight);
	voxel::Region region(0, 0, 0, image->width() - 1, maxHeight - 1, image->height() - 1);
	voxel::RawVolume *volume = new voxel::RawVolume(region);
	voxel::RawVolumeWrapper wrapper(volume);
	scenegraph::SceneGraphNode node(scenegraph::SceneGraphNodeType::Model);
	const voxel::Voxel dirtVoxel = voxel::createVoxel(voxel::VoxelType::Generic, 1);
	if (coloredHeightmap) {
		palette::PaletteLookup palLookup;
		voxelutil::importColoredHeightmap(wrapper, palLookup, image, dirtVoxel);
		node.setPalette(palLookup.palette());
	} else {
		const voxel::Voxel grassVoxel = voxel::createVoxel(voxel::VoxelType::Generic, 2);
		voxelutil::importHeightmap(wrapper, image, dirtVoxel, grassVoxel);
	}
	node.setVolume(volume, true);
	node.setName(core::string::extractFilename(filename));
	return sceneGraph.emplace(core::move(node)) != InvalidNodeId;
}

bool PNGFormat::importAsVolume(scenegraph::SceneGraph &sceneGraph, const palette::Palette &palette,
							   const core::String &filename, const io::ArchivePtr &archive) const {
	const image::ImagePtr &image = image::loadImage(filename);
	const int maxDepth = core::Var::getSafe(cfg::VoxformatImageVolumeMaxDepth)->intVal();
	const bool bothSides = core::Var::getSafe(cfg::VoxformatImageVolumeBothSides)->boolVal();
	const core::String &depthMapFilename = voxelutil::getDefaultDepthMapFile(filename);
	core::ScopedPtr<io::SeekableReadStream> depthMapStream(archive->readStream(depthMapFilename));
	const image::ImagePtr &depthMapImage = image::loadImage(depthMapFilename, *depthMapStream, depthMapStream->size());
	voxel::RawVolume *v;
	if (depthMapImage && depthMapImage->isLoaded()) {
		Log::debug("Found depth map %s", depthMapFilename.c_str());
		v = voxelutil::importAsVolume(image, depthMapImage, palette, maxDepth, bothSides);
	} else {
		Log::debug("Could not find a depth map for %s with the name %s", filename.c_str(), depthMapFilename.c_str());
		v = voxelutil::importAsVolume(image, palette, maxDepth, bothSides);
	}
	if (v == nullptr) {
		Log::warn("Failed to import image as volume: '%s'", image->name().c_str());
		return false;
	}
	scenegraph::SceneGraphNode node(scenegraph::SceneGraphNodeType::Model);
	node.setVolume(v, true);
	node.setName(core::string::extractFilename(filename));
	return sceneGraph.emplace(core::move(node)) != InvalidNodeId;
}

bool PNGFormat::importAsPlane(scenegraph::SceneGraph &sceneGraph, const palette::Palette &palette,
							  const core::String &filename, const io::ArchivePtr &archive) const {
	image::ImagePtr image = image::loadImage(filename);
	voxel::RawVolume *v = voxelutil::importAsPlane(image, palette);
	if (v == nullptr) {
		Log::warn("Failed to import image as plane: '%s'", image->name().c_str());
		return false;
	}
	scenegraph::SceneGraphNode node(scenegraph::SceneGraphNodeType::Model);
	node.setVolume(v, true);
	node.setName(core::string::extractFilename(filename));
	return sceneGraph.emplace(core::move(node)) != InvalidNodeId;
}

bool PNGFormat::loadGroupsRGBA(const core::String &filename, const io::ArchivePtr &archive,
							   scenegraph::SceneGraph &sceneGraph, const palette::Palette &palette,
							   const LoadContext &ctx) {
	const int type = core::Var::getSafe(cfg::VoxformatImageImportType)->intVal();
	if (type == 1) {
		return importAsHeightmap(sceneGraph, palette, filename, archive);
	}
	if (type == 2) {
		return importAsVolume(sceneGraph, palette, filename, archive);
	}

	core::String basename = core::string::extractFilename(filename);
	const core::String &directory = core::string::extractDir(filename);
	size_t sep = basename.rfind('-');
	if (sep != core::String::npos) {
		basename = basename.substr(0, sep);
	}
	Log::debug("Base name for image layer import is: %s", basename.c_str());

	io::ArchiveFiles entities;
	archive->list(directory, entities, core::string::format("%s-*.png", basename.c_str()));
	if (entities.empty()) {
		io::FilesystemEntry val = io::createFilesystemEntry(core::Path(filename));
		entities.push_back(val);
	}
	Log::debug("Found %i images for import", (int)entities.size());

	if (entities.size() > 1u) {
		return importSlices(sceneGraph, palette, entities);
	}
	return importAsPlane(sceneGraph, palette, filename, archive);
}

size_t PNGFormat::loadPalette(const core::String &filename, const io::ArchivePtr &archive, palette::Palette &palette,
							  const LoadContext &ctx) {
	// TODO:
	palette.nippon();
	return palette.colorCount();
}

bool PNGFormat::saveGroups(const scenegraph::SceneGraph &sceneGraph, const core::String &filename,
						   const io::ArchivePtr &archive, const SaveContext &ctx) {
	const core::String &basename = core::string::stripExtension(filename);
	for (const auto &e : sceneGraph.nodes()) {
		const scenegraph::SceneGraphNode &node = e->value;
		if (!node.isAnyModelNode()) {
			continue;
		}
		const voxel::RawVolume *volume = sceneGraph.resolveVolume(node);
		core_assert(volume != nullptr);
		const voxel::Region &region = volume->region();
		const palette::Palette &palette = node.palette();
		for (int z = region.getLowerZ(); z <= region.getUpperZ(); ++z) {
			const core::String &layerFilename = core::string::format("%s-%s-%i.png", basename.c_str(), node.uuid().c_str(), z);
			image::Image image(layerFilename);
			core::Buffer<core::RGBA> rgba(region.getWidthInVoxels() * region.getHeightInVoxels());
			for (int y = region.getUpperY(); y >= region.getLowerY(); --y) {
				for (int x = region.getLowerX(); x <= region.getUpperX(); ++x) {
					const voxel::Voxel &v = volume->voxel(x, y, z);
					if (voxel::isAir(v.getMaterial())) {
						continue;
					}
					const core::RGBA color = palette.color(v.getColor());
					const int idx = (region.getUpperY() - y) * region.getWidthInVoxels() + (x - region.getLowerX());
					rgba[idx] = color;
				}
			}
			if (!image.loadRGBA((const uint8_t *)rgba.data(), region.getWidthInVoxels(), region.getHeightInVoxels())) {
				Log::error("Failed to load sliced rgba data %s", layerFilename.c_str());
				return false;
			}
			if (!image::writeImage(image, layerFilename)) {
				Log::error("Failed to write slice image %s", layerFilename.c_str());
				return false;
			}
		}
	}
	return true;
}

} // namespace voxelformat
