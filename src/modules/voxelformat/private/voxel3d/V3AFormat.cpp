/**
 * @file
 */

#include "V3AFormat.h"
#include "core/Log.h"
#include "core/ScopedPtr.h"
#include "core/StringUtil.h"
#include "io/ZipReadStream.h"
#include "io/ZipWriteStream.h"
#include "scenegraph/SceneGraph.h"
#include "palette/Palette.h"
#include "palette/PaletteLookup.h"

#include <SDL3/SDL_stdinc.h>

namespace voxelformat {

#define wrap(read)                                                                                                     \
	if ((read) != 0) {                                                                                                 \
		Log::error("Could not load v3a file: Not enough data in stream " CORE_STRINGIFY(read));                        \
		return false;                                                                                                  \
	}

#define wrapBool(read)                                                                                                 \
	if ((read) != true) {                                                                                              \
		Log::error("Could not load v3a file: Not enough data in stream " CORE_STRINGIFY(read) " (line %i)",            \
				   (int)__LINE__);                                                                                     \
		return false;                                                                                                  \
	}

bool V3AFormat::loadGroupsRGBA(const core::String &filename, const io::ArchivePtr &archive,
							   scenegraph::SceneGraph &sceneGraph, const palette::Palette &palette,
							   const LoadContext &ctx) {
	core::ScopedPtr<io::SeekableReadStream> stream(archive->readStream(filename));
	if (!stream) {
		Log::error("Could not load file %s", filename.c_str());
		return false;
	}
	if (core::string::toLower(filename.last()) == 'b') {
		io::ZipReadStream zipStream(*stream);
		return loadFromStream(filename, &zipStream, sceneGraph, palette, ctx);
	}
	return loadFromStream(filename, stream, sceneGraph, palette, ctx);
}

bool V3AFormat::loadFromStream(const core::String &filename, io::ReadStream *stream,
							   scenegraph::SceneGraph &sceneGraph, const palette::Palette &palette,
							   const LoadContext &ctx) {
	core::String line;
	int lineCnt = 0;
	int width, depth, height;
	width = depth = height = 0;
	scenegraph::SceneGraphNode node;
	while (!stream->eos()) {
		wrapBool(stream->readLine(line));
		++lineCnt;
		if (core::string::startsWith(line, "VERSION")) {
			line = line.substr(8);
			if (line != "1.0") {
				Log::error("Unsupported VERSION: %s", line.c_str());
				return false;
			}
			node.setProperty("version", line);
			continue;
		}
		if (core::string::startsWith(line, "TYPE")) {
			line = line.substr(5);
			if (line != "VoxelCubic") {
				Log::error("Unsupported TYPE: %s", line.c_str());
				return false;
			}
			continue;
		}
		if (core::string::startsWith(line, "DIMENSION")) {
			continue;
		}
		if (core::string::startsWith(line, "SIZE")) {
			line = line.substr(5);
			if (SDL_sscanf(line.c_str(), "%i %i %i", &width, &depth, &height) != 3) {
				Log::error("Failed to parse SIZE line: %s", line.c_str());
				return false;
			}
			continue;
		}
		if (core::string::startsWith(line, "DATA")) {
			line = line.substr(5);
			break;
		}
		Log::warn("Unsupported line: %s", line.c_str());
	}

	if (width == 0 || height == 0 || depth == 0) {
		Log::error("Invalid size: %i:%i:%i", width, height, depth);
		return false;
	}

	if (width > 2048 || height > 2048 || depth > 2048) {
		Log::error("Volume exceeds the max allowed size: %i:%i:%i", width, height, depth);
		return false;
	}

	palette::PaletteLookup palLookup(palette);
	voxel::Region region(0, 0, 0, width, height, depth);
	voxel::RawVolume *volume = new voxel::RawVolume(region);
	node.setVolume(volume, true);

	core::DynamicArray<core::String> tokens;
	int x = 0;
	int y = 0;
	do {
		if (line.empty()) {
			++lineCnt;
			if (!stream->readLine(line)) {
				break;
			}
			y = 0;
			++x;
			if (x > width) {
				Log::error("Max width exceeded at line %i: x: %i, y: %i, width: %i, height: %i", lineCnt, x, y, width,
						   height);
				return false;
			}
		}

		if (y > height) {
			Log::error("Max depth exceeded at line %i: x: %i, y: %i, width: %i, height: %i", lineCnt, x, y, width,
					   height);
			return false;
		}
		tokens.clear();
		core::string::splitString(line, tokens);
		if (tokens.size() % 4 != 0) {
			Log::error("Invalid data line %i: %s", lineCnt, line.c_str());
			return false;
		}
		for (size_t i = 0; i < tokens.size(); i += 4) {
			const int r = core::string::toInt(tokens[i + 0]);
			if (r == -1) {
				continue;
			}
			const int g = core::string::toInt(tokens[i + 1]);
			const int b = core::string::toInt(tokens[i + 2]);
			const int a = core::string::toInt(tokens[i + 3]);
			const core::RGBA rgba(r, g, b, a);
			const int index = palLookup.findClosestIndex(rgba);
			const voxel::Voxel &voxel = voxel::createVoxel(palette, index);
			// we have to flip depth with height for our own coordinate system
			volume->setVoxel(x, y, (int)i / 4, voxel);
		}
		++y;
		++lineCnt;
	} while (stream->readLine(line));
	node.setName(filename);
	node.setPalette(palLookup.palette());
	sceneGraph.emplace(core::move(node));
	return true;
}

#undef wrap
#undef wrapBool

#define wrapBool(read)                                                                                                 \
	if ((read) != true) {                                                                                              \
		Log::error("Could not write v3a file: " CORE_STRINGIFY(read) " (line %i)", (int)__LINE__);                     \
		return false;                                                                                                  \
	}

bool V3AFormat::saveGroups(const scenegraph::SceneGraph &sceneGraph, const core::String &filename,
						   const io::ArchivePtr &archive, const SaveContext &ctx) {
	core::ScopedPtr<io::SeekableWriteStream> stream(archive->writeStream(filename));
	if (!stream) {
		Log::error("Could not open file %s", filename.c_str());
		return false;
	}
	if (core::string::toLower(filename.last()) == 'b') {
		io::ZipWriteStream zipStream(*stream);
		return saveToStream(sceneGraph, &zipStream, ctx);
	}
	return saveToStream(sceneGraph, stream, ctx);
}

bool V3AFormat::saveToStream(const scenegraph::SceneGraph &sceneGraph, io::WriteStream *stream,
							 const SaveContext &ctx) {
	const scenegraph::SceneGraphNode *node = sceneGraph.firstModelNode();
	core_assert(node);

	const voxel::Region &region = node->region();
	const uint32_t width = region.getWidthInVoxels();
	const uint32_t height = region.getHeightInVoxels();
	const uint32_t depth = region.getDepthInVoxels();

	wrapBool(stream->writeString("VERSION 1.0\r\n", false))
	wrapBool(stream->writeString("TYPE VoxelCubic\r\n", false))
	if (!stream->writeStringFormat(false, "DIMENSION %d.0 %d.0 %d.0\r\n", width, height, depth)) {
		Log::error("Failed to write DIMENSION line");
		return false;
	}
	if (!stream->writeStringFormat(false, "SIZE %d %d %d\r\n", width, height, depth)) {
		Log::error("Failed to write SIZE line");
		return false;
	}
	wrapBool(stream->writeString("DATA ", false))
	const voxel::RawVolume &volume = *node->volume();
	const palette::Palette &palette = node->palette();
	for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x++) {
		for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y++) {
			for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z++) {
				const voxel::Voxel &voxel = volume.voxel(x, y, z);
				if (voxel::isAir(voxel.getMaterial())) {
					wrapBool(stream->writeString("-1 -1 -1 -1 ", false))
				} else {
					const core::RGBA color = palette.color(voxel.getColor());
					if (!stream->writeStringFormat(false, "%d %d %d %d ", color.r, color.g, color.b, color.a)) {
						Log::error("Failed to write voxel data");
						return false;
					}
				}
			}
			wrapBool(stream->writeString("\r\n", false))
		}
		wrapBool(stream->writeString("\r\n", false))
	}

	return true;
}

#undef wrapBool

} // namespace voxelformat
