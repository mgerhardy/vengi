/**
 * @file
 */

#include "CubzhFormat.h"
#include "CubzhShared.h"
#include "core/ScopedPtr.h"
#include "core/StringUtil.h"
#include "io/Archive.h"
#include "io/ZipReadStream.h"
#include "palette/Palette.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/SceneGraphNode.h"

namespace voxelformat {

// TODO: z-axis must be flipped

#define wrap(read)                                                                                                     \
	if ((read) != 0) {                                                                                                 \
		Log::error("Could not load 3zh file: Not enough data in stream " CORE_STRINGIFY(read) " (line %i)",            \
				   (int)__LINE__);                                                                                     \
		return false;                                                                                                  \
	}

#define wrapBool(read)                                                                                                 \
	if ((read) != true) {                                                                                              \
		Log::error("Could not load 3zh file: Not enough data in stream " CORE_STRINGIFY(read) " (line %i)",            \
				   (int)__LINE__);                                                                                     \
		return false;                                                                                                  \
	}

bool CubzhFormat::Chunk::supportsCompression() const {
	return priv::supportsCompression(chunkId);
}

CubzhFormat::CubzhReadStream::CubzhReadStream(const Header &header, const Chunk &chunk,
											  io::SeekableReadStream &forward) {
	if (header.compressionType == 1 && chunk.compressed != 0) {
		Log::debug("load compressed chunk with id %d and size %d", chunk.chunkId, chunk.chunkSize);
		_stream = new io::ZipReadStream(forward, chunk.chunkSize);
		_forwarded = true;
		_size = chunk.uncompressedSize;
	} else {
		Log::debug("load uncompressed chunk with id %d and size %d", chunk.chunkId, chunk.chunkSize);
		_stream = &forward;
		_forwarded = false;
		_size = chunk.chunkSize;
	}
	_pos = 0;
}

CubzhFormat::CubzhReadStream::~CubzhReadStream() {
	if (_forwarded) {
		delete _stream;
	}
}

int CubzhFormat::CubzhReadStream::read(void *dataPtr, size_t dataSize) {
	if (dataSize > (size_t)remaining()) {
		Log::debug("requested to read %d bytes, but only %d are left", (int)dataSize, (int)remaining());
	}
	const int bytes = _stream->read(dataPtr, dataSize);
	if (bytes > 0) {
		_pos += bytes;
	}
	return bytes;
}

bool CubzhFormat::CubzhReadStream::eos() const {
	return pos() >= size();
}

int64_t CubzhFormat::CubzhReadStream::size() const {
	return _size;
}

int64_t CubzhFormat::CubzhReadStream::pos() const {
	return _pos;
}

int64_t CubzhFormat::CubzhReadStream::remaining() const {
	return size() - pos();
}

bool CubzhFormat::CubzhReadStream::empty() const {
	return size() == 0;
}

bool CubzhFormat::loadSkipChunk(const Header &header, const Chunk &chunk, io::ReadStream &stream) const {
	Log::debug("skip chunk %u with size %d", chunk.chunkId, (int)chunk.chunkSize);
	if (header.version == 6u && chunk.supportsCompression()) {
		Log::debug("skip additional header bytes for compressed chunk");
		// stream.skipDelta(5); // iscompressed byte and uncompressed size uint32
	}
	return stream.skipDelta(chunk.chunkSize) == 0;
}

bool CubzhFormat::loadSkipSubChunk(const Chunk &chunk, io::ReadStream &stream) const {
	Log::debug("skip subchunk %u", chunk.chunkId);
	return stream.skipDelta(chunk.chunkSize) == 0;
}

bool CubzhFormat::loadHeader(io::SeekableReadStream &stream, Header &header) const {
	uint8_t magic[6];
	if (stream.read(magic, sizeof(magic)) != (int)sizeof(magic)) {
		Log::error("Could not load 3zh magic: Not enough data in stream");
		return false;
	}

	if (!memcmp(magic, "CUBZH!", sizeof(magic))) {
		header.legacy = false;
		Log::debug("Found cubzh file");
	} else if (!memcmp(magic, "PARTIC", sizeof(magic))) {
		header.legacy = true;
		stream.skip(5); // UBES!
		Log::debug("Found particubes file");
	} else {
		Log::error("Could not load 3zh file: Invalid magic");
		return false;
	}

	wrap(stream.readUInt32(header.version))
	if (header.version != 5u && header.version != 6u) {
		Log::warn("Unsupported version %d", header.version);
	} else {
		Log::debug("Found version %d", header.version);
	}
	wrap(stream.readUInt8(header.compressionType));
	wrap(stream.readUInt32(header.totalSize))

	if (header.version == 5) {
		uint32_t uncompressedSize;
		wrap(stream.readUInt32(uncompressedSize))
	}

	Log::debug("CompressionType: %d", header.compressionType);
	Log::debug("Total size: %d", header.totalSize);

	return true;
}

bool CubzhFormat::loadPalettePCubes(io::ReadStream &stream, palette::Palette &palette) const {
	uint8_t colorCount;
	Log::debug("Found legacy palette");
	// rowCount
	// columnCount
	stream.skipDelta(2);
	uint16_t colorCount16;
	wrap(stream.readUInt16(colorCount16))
	colorCount = colorCount16;
	// default color
	// default background color
	stream.skipDelta(2);
	Log::debug("Palette with %d colors", colorCount);

	palette.setSize(colorCount);
	for (uint8_t i = 0; i < colorCount; ++i) {
		uint8_t r, g, b, a;
		wrap(stream.readUInt8(r))
		wrap(stream.readUInt8(g))
		wrap(stream.readUInt8(b))
		wrap(stream.readUInt8(a))
		palette.setColor(i, core::RGBA(r, g, b, a));
	}
	for (uint8_t i = 0; i < colorCount; ++i) {
		const bool emissive = stream.readBool();
		if (emissive) {
			palette.setEmit(i, 1.0f);
		}
	}
	return true;
}

bool CubzhFormat::loadPalette5(io::ReadStream &stream, palette::Palette &palette) const {
	uint8_t colorCount;
	Log::debug("Found v5 palette");
	uint8_t colorEncoding;
	wrap(stream.readUInt8(colorEncoding))
	if (colorEncoding != 1) {
		Log::error("Unsupported color encoding %d", colorEncoding);
		return false;
	}
	uint8_t rowCount;
	wrap(stream.readUInt8(rowCount))
	uint8_t columnCount;
	wrap(stream.readUInt8(columnCount))
	uint16_t colorCount16;
	wrap(stream.readUInt16(colorCount16))
	colorCount = colorCount16;

	if (colorCount16 != ((uint16_t)rowCount * (uint16_t)columnCount)) {
		Log::error("Invalid color count %d", colorCount16);
		return 0;
	}
	palette.setSize(colorCount);
	for (uint8_t i = 0; i < colorCount; ++i) {
		uint8_t r, g, b, a;
		wrap(stream.readUInt8(r))
		wrap(stream.readUInt8(g))
		wrap(stream.readUInt8(b))
		wrap(stream.readUInt8(a))
		palette.setColor(i, core::RGBA(r, g, b, a));
	}
	// default color
	// default background color
	stream.skipDelta(2);
	Log::debug("Palette with %d colors", colorCount);

	return true;
}

bool CubzhFormat::loadPalette6(io::ReadStream &stream, palette::Palette &palette) const {
	uint8_t colorCount = 0;
	wrap(stream.readUInt8(colorCount))
	Log::debug("Palette with %d colors", colorCount);

	palette.setSize(colorCount);
	for (uint8_t i = 0; i < colorCount; ++i) {
		uint8_t r, g, b, a;
		wrap(stream.readUInt8(r))
		wrap(stream.readUInt8(g))
		wrap(stream.readUInt8(b))
		wrap(stream.readUInt8(a))
		palette.setColor(i, core::RGBA(r, g, b, a));
	}
	for (uint8_t i = 0; i < colorCount; ++i) {
		const bool emissive = stream.readBool();
		if (emissive) {
			palette.setEmit(i, 1.0f);
		}
	}
	return true;
}

bool CubzhFormat::loadShape5(const core::String &filename, const Header &header, const Chunk &chunk, io::SeekableReadStream &stream,
							 scenegraph::SceneGraph &sceneGraph, const palette::Palette &palette,
							 const LoadContext &ctx) const {
	uint16_t width = 0, depth = 0, height = 0;
	scenegraph::SceneGraphNode node;

	int64_t startPos = stream.pos();
	while (stream.pos() < startPos + chunk.chunkSize - 5) {
		Chunk subChunk;
		wrapBool(loadSubChunkHeader(stream, subChunk))
		core::DynamicArray<uint8_t> volumeBuffer; // used in case the size chunk is late
		switch (subChunk.chunkId) {
		case priv::CHUNK_ID_SHAPE_SIZE_V5:
			wrap(stream.readUInt16(width))
			wrap(stream.readUInt16(height))
			wrap(stream.readUInt16(depth))
			Log::debug("Found size chunk: %i:%i:%i", width, height, depth);

			if (!volumeBuffer.empty()) {
				const voxel::Region region(0, 0, 0, (int)width - 1, (int)height - 1, (int)depth - 1);
				if (!region.isValid()) {
					Log::error("Invalid region: %i:%i:%i", width, height, depth);
					return false;
				}

				voxel::RawVolume *volume = new voxel::RawVolume(region);
				node.setVolume(volume, true);
				int i = 0;
				for (uint16_t x = 0; x < width; x++) {
					for (uint16_t y = 0; y < height; y++) {
						for (uint16_t z = 0; z < depth; z++) {
							const uint8_t index = volumeBuffer[i];
							if (index == emptyPaletteIndex()) {
								continue;
							}
							const voxel::Voxel &voxel = voxel::createVoxel(palette, index);
							volume->setVoxel(width - x - 1, y, z, voxel);
						}
					}
				}
			}
			break;
		case priv::CHUNK_ID_SHAPE_BLOCKS_V5: {
			Log::debug("Shape with %u voxels found", subChunk.chunkSize);
			if (width == 0) {
				volumeBuffer.reserve(subChunk.chunkSize);
				for (uint32_t i = 0; i < subChunk.chunkSize; ++i) {
					uint8_t index;
					wrap(stream.readUInt8(index))
					volumeBuffer.push_back(index);
				}
				break;
			}
			uint32_t voxelCount = (uint32_t)width * (uint32_t)height * (uint32_t)depth;
			if (voxelCount * sizeof(uint8_t) != subChunk.chunkSize) {
				Log::error("Invalid size for blocks chunk: %i", subChunk.chunkSize);
				return false;
			}
			const voxel::Region region(0, 0, 0, (int)width - 1, (int)height - 1, (int)depth - 1);
			if (!region.isValid()) {
				Log::error("Invalid region: %i:%i:%i", width, height, depth);
				return false;
			}

			voxel::RawVolume *volume = new voxel::RawVolume(region);
			node.setVolume(volume, true);
			for (uint32_t i = 0; i < voxelCount; i++) {
				uint8_t index;
				wrap(stream.readUInt8(index))
				if (index == emptyPaletteIndex()) {
					continue;
				}

				const uint32_t z = i / (width * height);
				const uint32_t y = (i - (uint32_t)(z * (width * height))) / width;
				const uint32_t x = i - (uint32_t)(z * (width * height)) - (uint32_t)(y * width);

				const voxel::Voxel &voxel = voxel::createVoxel(palette, index);
				volume->setVoxel(width - x - 1, y, z, voxel);
			}
			break;
		}
		case priv::CHUNK_ID_SHAPE_POINT_V5: {
			core::String name;
			wrapBool(stream.readString(subChunk.chunkSize, name))
			float f3x, f3y, f3z;
			wrap(stream.readFloat(f3x))
			wrap(stream.readFloat(f3y))
			wrap(stream.readFloat(f3z))
			node.setProperty(name, core::string::format("%f:%f:%f", f3x, f3y, f3z));
			break;
		}
		default:
			wrapBool(loadSkipSubChunk(subChunk, stream))
			break;
		}
	}
	if (node.volume() == nullptr) {
		Log::error("No volume found in v5 file");
		return false;
	}
	node.setName(filename);
	node.setPalette(palette);
	return sceneGraph.emplace(core::move(node)) != InvalidNodeId;
}

bool CubzhFormat::loadVersion5(const core::String &filename, const Header &header, io::SeekableReadStream &stream,
							   scenegraph::SceneGraph &sceneGraph, palette::Palette &palette,
							   const LoadContext &ctx) const {
	while (!stream.eos()) {
		Chunk chunk;
		wrapBool(loadChunkHeader(header, stream, chunk))
		ChunkChecker check(stream, chunk);
		switch (chunk.chunkId) {
		case priv::CHUNK_ID_PALETTE_V5:
			if (!loadPalette5(stream, palette)) {
				return false;
			}
			break;
		case priv::CHUNK_ID_SHAPE_V5:
			if (!loadShape5(filename, header, chunk, stream, sceneGraph, palette, ctx)) {
				return false;
			}
			break;
		default:
			wrapBool(loadSkipChunk(header, chunk, stream))
			break;
		}
	}

	return true;
}

bool CubzhFormat::loadPCubes(const core::String &filename, const Header &header, io::SeekableReadStream &stream,
							 scenegraph::SceneGraph &sceneGraph, palette::Palette &palette,
							 const LoadContext &ctx) const {
	while (!stream.eos()) {
		Chunk chunk;
		wrapBool(loadChunkHeader(header, stream, chunk))
		ChunkChecker check(stream, chunk);
		switch (chunk.chunkId) {
		case priv::CHUNK_ID_PALETTE_LEGACY_V6: {
			Log::debug("load palette");
			CubzhReadStream zhs(header, chunk, stream);
			wrapBool(loadPalettePCubes(zhs, palette))
			break;
		}
		case priv::CHUNK_ID_SHAPE_V6: {
			Log::debug("load shape");
			CubzhReadStream zhs(header, chunk, stream);
			wrapBool(loadShape6(filename, header, chunk, zhs, sceneGraph, palette, ctx))
			break;
		}
		default:
			Log::debug("Skip chunk %d", chunk.chunkId);
			wrapBool(loadSkipChunk(header, chunk, stream))
			break;
		}
	}
	return true;
}

bool CubzhFormat::loadChunkHeader(const Header &header, io::ReadStream &stream, Chunk &chunk) const {
	wrap(stream.readUInt8(chunk.chunkId))
	if (header.version == 6 && chunk.chunkId == priv::CHUNK_ID_SHAPE_NAME_V6) {
		uint8_t chunkSize;
		wrap(stream.readUInt8(chunkSize))
		chunk.chunkSize = chunkSize;
	} else {
		wrap(stream.readUInt32(chunk.chunkSize))
	}
	Log::debug("Chunk id %u with size %u", chunk.chunkId, chunk.chunkSize);
	if (header.version == 6u && chunk.supportsCompression()) {
		wrap(stream.readUInt8(chunk.compressed))
		wrap(stream.readUInt32(chunk.uncompressedSize))
		Log::debug("Compressed: %u", chunk.compressed);
		Log::debug("Uncompressed size: %u", chunk.uncompressedSize);
	}
	return true;
}

bool CubzhFormat::loadSubChunkHeader(io::ReadStream &stream, Chunk &chunk) const {
	wrap(stream.readUInt8(chunk.chunkId))
	wrap(stream.readUInt32(chunk.chunkSize))
	Log::debug("Subchunk id %u with size %u", chunk.chunkId, chunk.chunkSize);
	return true;
}

bool CubzhFormat::loadShape6(const core::String &filename, const Header &header, const Chunk &chunk, CubzhReadStream &stream,
							 scenegraph::SceneGraph &sceneGraph, const palette::Palette &palette,
							 const LoadContext &ctx) const {
	uint16_t width = 0, depth = 0, height = 0;
	scenegraph::SceneGraphNode node;
	node.setName(core::string::extractFilename(filename));
	uint16_t shapeId = 1;
	uint16_t parentShapeId = 0;
	glm::vec3 pivot{0.5f}; // default is center of shape
	glm::vec3 pos{0};
	glm::vec3 eulerAngles{0};
	glm::vec3 scale{1};
	palette::Palette nodePalette = palette;
	bool hasPivot = false;
	bool sizeChunkFound = false;
	bool paletteFound = false;
	while (!stream.eos()) {
		Log::debug("Remaining sub stream data: %d", (int)stream.remaining());
		Chunk subChunk;
		wrapBool(loadSubChunkHeader(stream, subChunk))
		core::DynamicArray<uint8_t> volumeBuffer; // used in case the size chunk is late
		switch (subChunk.chunkId) {
		case priv::CHUNK_ID_SHAPE_ID_V6:
			wrap(stream.readUInt16(shapeId))
			Log::debug("Load shape id %u", shapeId);
			node.setProperty("shapeId", core::string::format("%d", shapeId));
			break;
		case priv::CHUNK_ID_SHAPE_PARENT_ID_V6:
			wrap(stream.readUInt16(parentShapeId))
			Log::debug("Load parent id %u", parentShapeId);
			break;
		case priv::CHUNK_ID_SHAPE_TRANSFORM_V6: {
			Log::debug("Load transform");
			wrap(stream.readFloat(pos.x))
			wrap(stream.readFloat(pos.y))
			wrap(stream.readFloat(pos.z))
			wrap(stream.readFloat(eulerAngles.x))
			wrap(stream.readFloat(eulerAngles.y))
			wrap(stream.readFloat(eulerAngles.z))
			wrap(stream.readFloat(scale.x))
			wrap(stream.readFloat(scale.y))
			wrap(stream.readFloat(scale.z))
			break;
		}
		case priv::CHUNK_ID_SHAPE_PIVOT_V6: {
			Log::debug("Load pivot");
			wrap(stream.readFloat(pivot.x))
			wrap(stream.readFloat(pivot.y))
			wrap(stream.readFloat(pivot.z))
			hasPivot = true;
			Log::debug("pivot: %f:%f:%f", pivot.x, pivot.y, pivot.z);
			break;
		}
		case priv::CHUNK_ID_SHAPE_PALETTE_V6: {
			wrapBool(loadPalette6(stream, nodePalette))
			paletteFound = true;
			break;
		}
		case priv::CHUNK_ID_OBJECT_COLLISION_BOX_V6: {
			Log::debug("Load collision box");
			glm::vec3 mins;
			glm::vec3 maxs;
			wrap(stream.readFloat(mins.x))
			wrap(stream.readFloat(mins.y))
			wrap(stream.readFloat(mins.z))
			wrap(stream.readFloat(maxs.x))
			wrap(stream.readFloat(maxs.y))
			wrap(stream.readFloat(maxs.z))
			break;
		}
		case priv::CHUNK_ID_OBJECT_IS_HIDDEN_V6: {
			Log::debug("Load hidden state");
			node.setVisible(!stream.readBool());
			break;
		}
		case priv::CHUNK_ID_SHAPE_NAME_V6: {
			core::String name;
			stream.readString(subChunk.chunkSize, name);
			if (!name.empty()) {
				node.setName(name);
			}
			Log::debug("Load node name: %s", name.c_str());
			break;
		}
		case priv::CHUNK_ID_SHAPE_SIZE_V6:
			Log::debug("Load shape size");
			wrap(stream.readUInt16(width))
			wrap(stream.readUInt16(height))
			wrap(stream.readUInt16(depth))
			Log::debug("Found size chunk: %i:%i:%i", width, height, depth);
			sizeChunkFound = true;

			if (!volumeBuffer.empty()) {
				const voxel::Region region(0, 0, 0, (int)width - 1, (int)height - 1, (int)depth - 1);
				if (!region.isValid()) {
					Log::error("Invalid region: %i:%i:%i", width, height, depth);
					return false;
				}

				voxel::RawVolume *volume = new voxel::RawVolume(region);
				node.setVolume(volume, true);
				int i = 0;
				for (uint16_t x = 0; x < width; x++) {
					for (uint16_t y = 0; y < height; y++) {
						for (uint16_t z = 0; z < depth; z++) {
							const uint8_t index = volumeBuffer[i];
							if (index == emptyPaletteIndex()) {
								continue;
							}
							const voxel::Voxel &voxel = voxel::createVoxel(palette, index);
							volume->setVoxel(width - x - 1, y, z, voxel);
						}
					}
				}
			}
			break;
		case priv::CHUNK_ID_SHAPE_BLOCKS_V6: {
			Log::debug("Shape with %u voxels found", subChunk.chunkSize);
			if (width == 0) {
				volumeBuffer.reserve(subChunk.chunkSize);
				for (uint32_t i = 0; i < subChunk.chunkSize; ++i) {
					uint8_t index;
					wrap(stream.readUInt8(index))
					volumeBuffer.push_back(index);
				}
				break;
			}
			uint32_t voxelCount = (uint32_t)width * (uint32_t)height * (uint32_t)depth;
			if (voxelCount * sizeof(uint8_t) != subChunk.chunkSize) {
				Log::error("Invalid size for blocks chunk: %i", subChunk.chunkSize);
				return false;
			}
			const voxel::Region region(0, 0, 0, (int)width - 1, (int)height - 1, (int)depth - 1);
			if (!region.isValid()) {
				Log::error("Invalid region: %i:%i:%i", width, height, depth);
				return false;
			}

			voxel::RawVolume *volume = new voxel::RawVolume(region);
			node.setVolume(volume, true);
			for (uint16_t x = 0; x < width; x++) {
				for (uint16_t y = 0; y < height; y++) {
					for (uint16_t z = 0; z < depth; z++) {
						uint8_t index;
						wrap(stream.readUInt8(index))
						if (index == emptyPaletteIndex()) {
							continue;
						}
						const voxel::Voxel &voxel = voxel::createVoxel(palette, index);
						volume->setVoxel(width - x - 1, y, z, voxel);
					}
				}
			}
			break;
		}
		case priv::CHUNK_ID_SHAPE_POINT_V6: {
			Log::debug("Load shape point");
			core::String name;
			wrapBool(stream.readPascalStringUInt8(name))
			float f3x, f3y, f3z;
			wrap(stream.readFloat(f3x))
			wrap(stream.readFloat(f3y))
			wrap(stream.readFloat(f3z))
			node.setProperty(name, core::string::format("%f:%f:%f", f3x, f3y, f3z));
			break;
		}
		case priv::CHUNK_ID_SHAPE_POINT_ROTATION_V6: {
			Log::debug("Load shape point rotation");
			core::String name;
			wrapBool(stream.readPascalStringUInt8(name))
			glm::vec3 poiPos;
			wrap(stream.readFloat(poiPos.x))
			wrap(stream.readFloat(poiPos.y))
			wrap(stream.readFloat(poiPos.z))
			break;
		}
		case priv::CHUNK_ID_SHAPE_BAKED_LIGHTING_V6:
		default:
			Log::debug("Ignore subchunk %u", subChunk.chunkId);
			wrapBool(loadSkipSubChunk(subChunk, stream))
			break;
		}
	}

	if (node.volume() == nullptr) {
		if (sizeChunkFound) {
			node.setVolume(new voxel::RawVolume(voxel::Region(0, 0)), true);
		} else {
			Log::error("No volume found");
			return false;
		}
	}
	scenegraph::SceneGraphTransform transform;
	transform.setLocalTranslation(pos);
	transform.setLocalOrientation(glm::quat(eulerAngles));
	transform.setLocalScale(scale);
	scenegraph::KeyFrameIndex keyFrameIdx = 0;
	node.setTransform(keyFrameIdx, transform);
	if (hasPivot) {
		pivot.x /= (float)width;
		pivot.y /= (float)height;
		pivot.z /= (float)depth;
	}
	node.setPivot(pivot);
	node.setPalette(nodePalette);
	int parent = 0;
	if (parentShapeId != 0) {
		if (scenegraph::SceneGraphNode *parentNode = sceneGraph.findNodeByPropertyValue("shapeId", core::string::format("%d", parentShapeId))) {
			parent = parentNode->id();
			if (!paletteFound) {
				node.setPalette(parentNode->palette());
			}
		} else {
			Log::warn("Could not find node with parent shape id %d", parentShapeId);
		}
	}
	return sceneGraph.emplace(core::move(node), parent) != InvalidNodeId;
}

bool CubzhFormat::loadVersion6(const core::String &filename, const Header &header, io::SeekableReadStream &stream,
							   scenegraph::SceneGraph &sceneGraph, palette::Palette &palette,
							   const LoadContext &ctx) const {
	while (!stream.eos()) {
		Log::debug("Remaining stream data: %d", (int)stream.remaining());
		Chunk chunk;
		wrapBool(loadChunkHeader(header, stream, chunk))
		ChunkChecker check(stream, chunk);
		if (chunk.chunkId < priv::CHUNK_ID_MIN || chunk.chunkId > priv::CHUNK_ID_MAX_V6) {
			Log::warn("Invalid chunk id found: %u", chunk.chunkId);
			break;
		}
		switch (chunk.chunkId) {
		case priv::CHUNK_ID_PALETTE_V6: {
			Log::debug("load v6 palette");
			CubzhReadStream zhs(header, chunk, stream);
			wrapBool(loadPalette6(zhs, palette))
			break;
		}
		case priv::CHUNK_ID_PALETTE_LEGACY_V6: {
			Log::debug("load legacy palette");
			CubzhReadStream zhs(header, chunk, stream);
			wrapBool(loadPalette5(zhs, palette))
			break;
		}
		case priv::CHUNK_ID_SHAPE_V6: {
			Log::debug("load shape");
			CubzhReadStream zhs(header, chunk, stream);
			wrapBool(loadShape6(filename, header, chunk, zhs, sceneGraph, palette, ctx))
			break;
		}
		case priv::CHUNK_ID_CAMERA_V6: {
			Log::debug("ignore camera");
			wrapBool(loadSkipChunk(header, chunk, stream))
			break;
		}
		case priv::CHUNK_ID_GENERAL_RENDERING_OPTIONS_V6: {
			Log::debug("ignore rendering options");
			wrapBool(loadSkipChunk(header, chunk, stream))
			break;
		}
		case priv::CHUNK_ID_DIRECTIONAL_LIGHT_V6: {
			Log::debug("ignore directional light");
			wrapBool(loadSkipChunk(header, chunk, stream))
			break;
		}
		default:
			Log::debug("ignore chunk with id %i", chunk.chunkId);
			wrapBool(loadSkipChunk(header, chunk, stream))
			break;
		}
	}

	return true;
}

bool CubzhFormat::loadGroupsPalette(const core::String &filename, const io::ArchivePtr &archive,
									scenegraph::SceneGraph &sceneGraph, palette::Palette &palette,
									const LoadContext &ctx) {
	core::ScopedPtr<io::SeekableReadStream> stream(archive->readStream(filename));
	if (!stream) {
		Log::error("Could not open file %s", filename.c_str());
		return 0;
	}
	Header header;
	wrapBool(loadHeader(*stream, header))
	Log::debug("Found version %d", header.version);
	if (header.legacy) {
		return loadPCubes(filename, header, *stream, sceneGraph, palette, ctx);
	}
	if (header.version == 5) {
		return loadVersion5(filename, header, *stream, sceneGraph, palette, ctx);
	}
	return loadVersion6(filename, header, *stream, sceneGraph, palette, ctx);
}

size_t CubzhFormat::loadPalette(const core::String &filename, const io::ArchivePtr &archive, palette::Palette &palette,
								const LoadContext &ctx) {
	core::ScopedPtr<io::SeekableReadStream> stream(archive->readStream(filename));
	if (!stream) {
		Log::error("Could not open file %s", filename.c_str());
		return 0;
	}
	Header header;
	wrapBool(loadHeader(*stream, header))
	while (!stream->eos()) {
		Chunk chunk;
		wrapBool(loadChunkHeader(header, *stream, chunk))
		ChunkChecker check(*stream, chunk);
		if (header.version == 5u && chunk.chunkId == priv::CHUNK_ID_PALETTE_V5) {
			wrapBool(loadPalettePCubes(*stream, palette))
			return palette.size();
		} else if (header.version == 6u && chunk.chunkId == priv::CHUNK_ID_PALETTE_V6) {
			CubzhReadStream zhs(header, chunk, *stream);
			wrapBool(loadPalette6(zhs, palette))
			return palette.size();
		} else if (header.version == 6u && chunk.chunkId == priv::CHUNK_ID_PALETTE_LEGACY_V6) {
			CubzhReadStream zhs(header, chunk, *stream);
			wrapBool(loadPalettePCubes(zhs, palette))
			return palette.size();
		} else {
			wrapBool(loadSkipChunk(header, chunk, *stream))
		}
	}
	return palette.size();
}

image::ImagePtr CubzhFormat::loadScreenshot(const core::String &filename, const io::ArchivePtr &archive,
											const LoadContext &ctx) {
	core::ScopedPtr<io::SeekableReadStream> stream(archive->readStream(filename));
	if (!stream) {
		Log::error("Could not open file %s", filename.c_str());
		return image::ImagePtr();
	}
	Header header;
	if (!loadHeader(*stream, header)) {
		Log::error("Failed to read header");
		return image::ImagePtr();
	}
	while (!stream->eos()) {
		Chunk chunk;
		if (!loadChunkHeader(header, *stream, chunk)) {
			return image::ImagePtr();
		}
		if (chunk.chunkId == priv::CHUNK_ID_PREVIEW) {
			image::ImagePtr img = image::createEmptyImage(core::string::extractFilename(filename) + ".png");
			img->load(*stream, chunk.chunkSize);
			return img;
		}
		if (!loadSkipChunk(header, chunk, *stream)) {
			Log::error("Failed to skip chunk %d with size %d", chunk.chunkId, chunk.chunkSize);
			break;
		}
	}
	return image::ImagePtr();
}

#undef wrap
#undef wrapBool

#define wrapBool(read)                                                                                                 \
	if (!(read)) {                                                                                                     \
		Log::error("Could not save 3zh file: Not enough data in stream " CORE_STRINGIFY(read));                        \
		return false;                                                                                                  \
	}

bool CubzhFormat::saveGroups(const scenegraph::SceneGraph &sceneGraph, const core::String &filename,
							 const io::ArchivePtr &archive, const SaveContext &ctx) {
	core::ScopedPtr<io::SeekableWriteStream> stream(archive->writeStream(filename));
	if (!stream) {
		Log::error("Could not open file %s", filename.c_str());
		return false;
	}
	stream->write("CUBZH!", 6);
	wrapBool(stream->writeUInt32(6)) // version
	wrapBool(stream->writeUInt8(1))	// zip compression
	const int64_t totalSizePos = stream->pos();
	wrapBool(stream->writeUInt32(0)) // total size is written at the end
	const int64_t afterHeaderPos = stream->pos();

	ThumbnailContext thumbnailCtx;
	thumbnailCtx.outputSize = glm::ivec2(128);
	const image::ImagePtr &image = createThumbnail(sceneGraph, ctx.thumbnailCreator, thumbnailCtx);
	if (image) {
		WriteChunkStream ws(priv::CHUNK_ID_PREVIEW, *stream);
		image->writePng(ws);
	}

	{
		WriteChunkStream ws(priv::CHUNK_ID_PALETTE_V6, *stream);
		const palette::Palette &palette = sceneGraph.firstPalette();
		const uint8_t colorCount = palette.colorCount();
		ws.writeUInt8(colorCount);
		for (uint8_t i = 0; i < colorCount; ++i) {
			const core::RGBA rgba = palette.color(i);
			wrapBool(ws.writeUInt8(rgba.r))
			wrapBool(ws.writeUInt8(rgba.g))
			wrapBool(ws.writeUInt8(rgba.b))
			wrapBool(ws.writeUInt8(rgba.a))
		}
		for (uint8_t i = 0; i < colorCount; ++i) {
			wrapBool(ws.writeBool(palette.hasEmit(i)))
		}
	}
	for (auto entry : sceneGraph.nodes()) {
		const scenegraph::SceneGraphNode &node = entry->second;
		if (!node.isAnyModelNode()) {
			continue;
		}
		WriteChunkStream ws(priv::CHUNK_ID_SHAPE_V6, *stream);
		{
			WriteSubChunkStream sub(priv::CHUNK_ID_SHAPE_ID_V6, ws);
			wrapBool(sub.writeUInt16(node.id()))
		}
		if (node.parent() != sceneGraph.root().id()) {
			WriteSubChunkStream sub(priv::CHUNK_ID_SHAPE_PARENT_ID_V6, ws);
			wrapBool(sub.writeUInt16(node.parent()))
		}
		{
			WriteSubChunkStream sub(priv::CHUNK_ID_SHAPE_TRANSFORM_V6, ws);
			scenegraph::KeyFrameIndex keyFrameIdx = 0;
			const scenegraph::SceneGraphTransform &transform = node.transform(keyFrameIdx);
			const glm::vec3 pos = transform.localTranslation();
			const glm::vec3 eulerAngles = glm::eulerAngles(transform.localOrientation());
			const glm::vec3 scale = transform.localScale();
			wrapBool(sub.writeFloat(pos.x))
			wrapBool(sub.writeFloat(pos.y))
			wrapBool(sub.writeFloat(pos.z))
			wrapBool(sub.writeFloat(eulerAngles.x))
			wrapBool(sub.writeFloat(eulerAngles.y))
			wrapBool(sub.writeFloat(eulerAngles.z))
			wrapBool(sub.writeFloat(scale.x))
			wrapBool(sub.writeFloat(scale.y))
			wrapBool(sub.writeFloat(scale.z))
		}
		{
			WriteSubChunkStream sub(priv::CHUNK_ID_SHAPE_PIVOT_V6, ws);
			const glm::vec3 &pivot = node.worldPivot();
			wrapBool(sub.writeFloat(pivot.x))
			wrapBool(sub.writeFloat(pivot.y))
			wrapBool(sub.writeFloat(pivot.z))
		}
		if (node.palette().colorCount() > 0) {
			WriteSubChunkStream sub(priv::CHUNK_ID_SHAPE_PALETTE_V6, ws);
			const palette::Palette &palette = node.palette();
			const uint8_t colorCount = palette.colorCount();
			sub.writeUInt8(colorCount);
			for (uint8_t i = 0; i < colorCount; ++i) {
				const core::RGBA rgba = palette.color(i);
				wrapBool(sub.writeUInt8(rgba.r))
				wrapBool(sub.writeUInt8(rgba.g))
				wrapBool(sub.writeUInt8(rgba.b))
				wrapBool(sub.writeUInt8(rgba.a))
			}
			for (uint8_t i = 0; i < colorCount; ++i) {
				wrapBool(sub.writeBool(palette.hasEmit(i)))
			}
		}
		{
			WriteSubChunkStream sub(priv::CHUNK_ID_OBJECT_COLLISION_BOX_V6, ws);
			const voxel::Region &region = node.region();
			const glm::ivec3 mins = region.getLowerCorner();
			const glm::ivec3 maxs = region.getUpperCorner() + 1;
			wrapBool(sub.writeFloat(mins.x))
			wrapBool(sub.writeFloat(mins.y))
			wrapBool(sub.writeFloat(mins.z))
			wrapBool(sub.writeFloat(maxs.x))
			wrapBool(sub.writeFloat(maxs.y))
			wrapBool(sub.writeFloat(maxs.z))
		}
		{
			WriteSubChunkStream sub(priv::CHUNK_ID_OBJECT_IS_HIDDEN_V6, ws);
			sub.writeBool(!node.visible());
		}
		{
			WriteSubChunkStream sub(priv::CHUNK_ID_SHAPE_SIZE_V6, ws);
			const glm::ivec3 &dimensions = node.region().getDimensionsInVoxels();
			wrapBool(sub.writeUInt16(dimensions.x))
			wrapBool(sub.writeUInt16(dimensions.y))
			wrapBool(sub.writeUInt16(dimensions.z))
		}
		{
			WriteSubChunkStream sub(priv::CHUNK_ID_SHAPE_BLOCKS_V6, ws);
			const voxel::RawVolume *volume = sceneGraph.resolveVolume(node);
			const voxel::Region &region = volume->region();
			const uint8_t emptyColorIndex = (uint8_t)emptyPaletteIndex();
			for (int x = region.getUpperX(); x >= region.getLowerX(); x--) {
				for (int y = region.getLowerY(); y <= region.getUpperY(); y++) {
					for (int z = region.getLowerZ(); z <= region.getUpperZ(); z++) {
						const voxel::Voxel &voxel = volume->voxel(x, y, z);
						if (voxel::isAir(voxel.getMaterial())) {
							wrapBool(sub.writeUInt8(emptyColorIndex))
						} else {
							wrapBool(sub.writeUInt8(voxel.getColor()))
						}
					}
				}
			}
		}
#if 0
		{
			core::String name = "TODO";
			glm::vec3 pos;
			core::string::parseVec3(node.property(name), &pos[0]);
			wrapBool(ws.writeUInt8(priv::CHUNK_ID_SHAPE_POINT_V6))
			wrapBool(ws.writePascalStringUInt8(name))
			wrapBool(ws.writeFloat(pos.x))
			wrapBool(ws.writeFloat(pos.y))
			wrapBool(ws.writeFloat(pos.z))
		}
#endif
#if 0
		{
			core::String name = "TODO";
			glm::vec3 poiPos;
			wrapBool(ws.writeUInt8(priv::CHUNK_ID_SHAPE_POINT_ROTATION_V6))
			wrapBool(ws.writePascalStringUInt8(name))
			wrapBool(ws.writeFloat(poiPos.x))
			wrapBool(ws.writeFloat(poiPos.y))
			wrapBool(ws.writeFloat(poiPos.z))
		}
#endif
		if (!node.name().empty()) {
			ws.writeUInt8(priv::CHUNK_ID_SHAPE_NAME_V6);
			ws.writePascalStringUInt8(node.name());
		}
	}

	const uint32_t totalSize = stream->size() - afterHeaderPos;
	if (stream->seek(totalSizePos) == -1) {
		Log::error("Failed to seek to the total size position in the header");
		return false;
	}
	wrapBool(stream->writeUInt32(totalSize))
	stream->seek(0, SEEK_END);
	return true;
}

#undef wrapBool

} // namespace voxelformat
