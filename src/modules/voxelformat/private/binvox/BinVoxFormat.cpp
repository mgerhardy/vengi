/**
 * @file
 */

#include "BinVoxFormat.h"
#include "core/Color.h"
#include "core/Log.h"
#include "core/ScopedPtr.h"
#include "core/StringUtil.h"
#include "io/Archive.h"
#include "io/Stream.h"
#include "scenegraph/SceneGraph.h"
#include "palette/Palette.h"

namespace voxelformat {

#define wrap(read)                                                                                                     \
	if ((read) != 0) {                                                                                                 \
		Log::error("Could not load binvox file: Not enough data in stream " CORE_STRINGIFY(read));                     \
		return false;                                                                                                  \
	}

#define wrapBool(read)                                                                                                 \
	if (!(read)) {                                                                                                     \
		Log::debug("Error: " CORE_STRINGIFY(read) " at " CORE_FILE ":%i", CORE_LINE);                                  \
		return false;                                                                                                  \
	}

bool BinVoxFormat::readData(State &state, const core::String &filename, io::SeekableReadStream &stream,
							scenegraph::SceneGraph &sceneGraph) {
	const voxel::Region region(0, 0, 0, (int)state._d - 1, (int)state._w - 1, (int)state._h - 1);
	if (!region.isValid()) {
		Log::error("Invalid region found in file");
		return false;
	}

	voxel::RawVolume *volume = new voxel::RawVolume(region);
	scenegraph::SceneGraphNode node;
	node.setVolume(volume, true);
	node.setName(filename);
	sceneGraph.emplace(core::move(node));
	const uint32_t numVoxels = state._w * state._h * state._d;
	uint32_t index = 0;
	uint32_t endIndex = 0;
	const palette::Palette &palette = node.palette();
	while (endIndex < numVoxels) {
		uint8_t value;
		uint8_t count;
		wrap(stream.readUInt8(value))
		wrap(stream.readUInt8(count))
		endIndex = index + count;
		if (endIndex > numVoxels) {
			Log::error("Given count is out of bounds");
			return false;
		}
		if (value != 0u) {
			const voxel::Voxel voxel = voxel::createVoxel(palette, value);
			for (uint32_t i = index; i < endIndex; ++i) {
				const int32_t iy = (int32_t)(i % state._w);
				const int32_t iz = (int32_t)((i / state._w) % state._h);
				const int32_t ix = (int32_t)(i / (state._w * state._h));
				if (!volume->setVoxel(ix, iy, iz, voxel)) {
					Log::debug("Failed to store voxel at x: %i, y: %i, z: %i (region: %s)", ix, iy, iz,
							   region.toString().c_str());
				}
			}
		}
		index = endIndex;
	}
	return true;
}

bool BinVoxFormat::loadGroups(const core::String &filename, const io::ArchivePtr &archive,
							  scenegraph::SceneGraph &sceneGraph, const LoadContext &ctx) {
	char line[512];

	core::ScopedPtr<io::SeekableReadStream> stream(archive->readStream(filename));
	if (!stream) {
		Log::error("Failed to open stream for file: %s", filename.c_str());
		return false;
	}

	wrapBool(stream->readLine(sizeof(line), line))
	if (0 != strcmp(line, "#binvox 1")) {
		Log::error("Expected to get '#binvox 1', but got '%s'", line);
		return false;
	}

	State state;
	if (SDL_sscanf(line, "#binvox %u", &state._version) != 1) {
		Log::error("Failed to parse binvox version");
		return false;
	}

	for (;;) {
		wrapBool(stream->readLine(sizeof(line), line))
		if (core::string::startsWith(line, "dim ")) {
			if (SDL_sscanf(line, "dim %u %u %u", &state._d, &state._h, &state._w) != 3) {
				Log::error("Failed to parse binvox dimensions");
				return false;
			}
		} else if (core::string::startsWith(line, "translate ")) {
			float tx, ty, tz;
			if (SDL_sscanf(line, "translate %f %f %f", &tz, &ty, &tx) != 3) {
				Log::error("Failed to parse binvox translation");
				return false;
			}
			state._tx = -tx;
			state._ty = -ty;
			state._tz = -tz;
		} else if (core::string::startsWith(line, "scale ")) {
			if (SDL_sscanf(line, "scale %f", &state._scale) != 1) {
				Log::error("Failed to parse binvox scale");
				return false;
			}
		} else if (core::string::startsWith(line, "data")) {
			break;
		} else {
			Log::error("unknown line: '%s'", line);
			return false;
		}
	}
	if (!readData(state, filename, *stream, sceneGraph)) {
		Log::warn("Could not load the data from %s", filename.c_str());
		return false;
	}
	sceneGraph.updateTransforms();
	return true;
}

bool BinVoxFormat::saveGroups(const scenegraph::SceneGraph &sceneGraph, const core::String &filename,
							  const io::ArchivePtr &archive, const SaveContext &ctx) {
	core::ScopedPtr<io::SeekableWriteStream> stream(archive->writeStream(filename));
	if (!stream) {
		Log::error("Failed to open stream for file: %s", filename.c_str());
		return false;
	}
	const scenegraph::SceneGraphNode *node = sceneGraph.firstModelNode();
	core_assert(node);
	const voxel::Region &region = node->region();
	voxel::RawVolume::Sampler sampler(node->volume());

	const int32_t width = region.getWidthInVoxels();
	const int32_t height = region.getHeightInVoxels();
	const int32_t depth = region.getDepthInVoxels();
	const glm::ivec3 mins = region.getLowerCorner();
	const glm::ivec3 maxs = region.getUpperCorner();
	const glm::ivec3 &offset = -mins;
	const float scale = 1.0f;

	wrapBool(stream->writeString("#binvox 1\n", false))
	stream->writeStringFormat(false, "dim %u %u %u\n", width, depth, height);
	stream->writeStringFormat(false, "translate %i %i %i\n", offset.x, offset.y, offset.z);
	stream->writeStringFormat(false, "scale %f\n", scale);
	wrapBool(stream->writeString("data\n", false))

	uint8_t count = 0u;
	uint8_t value = 0u;
	uint32_t voxels = 0u;
	const int maxIndex = width * height * depth;
	glm::ivec3 pos = mins;
	const uint8_t emptyColorReplacement = node->palette().findReplacement(0);
	Log::debug("found replacement for %s at index %u: %s at index %u",
			   core::Color::print(node->palette().color(0)).c_str(), 0,
			   core::Color::print(node->palette().color(emptyColorReplacement)).c_str(), emptyColorReplacement);
	for (int idx = 0; idx < maxIndex; ++idx) {
		if (!sampler.setPosition(pos)) {
			Log::error("Failed to set position for index %i (%i:%i:%i) (w:%i,h:%i,d:%i)", idx, pos.x, pos.y, pos.z,
					   width, height, depth);
			return false;
		}
		const voxel::Voxel voxel = sampler.voxel();
		if (isAir(voxel.getMaterial())) {
			if (value != 0u || count == 255u) {
				if (count > 0u) {
					wrapBool(stream->writeUInt8(value))
					wrapBool(stream->writeUInt8(count))
				}
				voxels += count;
				count = 0u;
			}
			++count;
			value = 0u;
		} else {
			uint8_t v = voxel.getColor();
			if (v == 0) {
				v = emptyColorReplacement;
			}
			if (value != v || count == 255u) {
				if (count > 0u) {
					wrapBool(stream->writeUInt8(value))
					wrapBool(stream->writeUInt8(count))
				}
				voxels += count;
				count = 0u;
			}
			++count;
			value = v;
		}
		++pos.y;
		if (pos.y > maxs.y) {
			pos.y = mins.y;
			++pos.z;
		}
		if (pos.z > maxs.z) {
			++pos.x;
			pos.y = mins.y;
			pos.z = mins.z;
		}
	}
	core_assert_msg(count > 0u, "Expected to have at least one voxel left: %i", (int)count);
	wrapBool(stream->writeUInt8(value))
	wrapBool(stream->writeUInt8(count))
	voxels += count;
	const uint32_t expectedVoxels = width * height * depth;
	if (voxels != expectedVoxels) {
		Log::error("Not enough data was written: %u vs %u (w: %u, h: %u, d: %u)", voxels, expectedVoxels, width, height,
				   depth);
		return false;
	}
	return true;
}

#undef wrap
#undef wrapBool

} // namespace voxelformat
