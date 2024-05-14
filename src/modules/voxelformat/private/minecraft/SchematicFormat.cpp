/**
 * @file
 */

#include "SchematicFormat.h"
#include "core/Color.h"
#include "core/Common.h"
#include "core/Log.h"
#include "core/ScopedPtr.h"
#include "core/StringUtil.h"
#include "core/collection/DynamicArray.h"
#include "io/ZipReadStream.h"
#include "io/ZipWriteStream.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/SceneGraphNode.h"
#include "voxel/MaterialColor.h"
#include "palette/Palette.h"
#include "palette/PaletteLookup.h"
#include "voxel/RawVolume.h"
#include "voxel/Region.h"
#include "voxel/Voxel.h"
#include "MinecraftPaletteMap.h"
#include "NamedBinaryTag.h"
#include "SchematicIntReader.h"

#include <glm/common.hpp>

namespace voxelformat {

static glm::ivec3 parsePosList(const priv::NamedBinaryTag &compound, const core::String &key) {
	const priv::NamedBinaryTag &pos = compound.get(key);
	int x = -1;
	int y = -1;
	int z = -1;
	if (pos.type() == priv::TagType::LIST) {
		const priv::NBTList &positions = *pos.list();
		if (positions.size() != 3) {
			Log::error("Unexpected nbt %s list entry count: %i", key.c_str(), (int)positions.size());
			return glm::ivec3(-1);
		}
		x = positions[0].int32(-1);
		y = positions[1].int32(-1);
		z = positions[2].int32(-1);
	} else if (pos.type() == priv::TagType::COMPOUND) {
		x = pos.get("x").int32(-1);
		y = pos.get("y").int32(-1);
		z = pos.get("z").int32(-1);
	}
	return glm::ivec3(x, y, z);
}

bool SchematicFormat::loadGroupsPalette(const core::String &filename, const io::ArchivePtr &archive,
										scenegraph::SceneGraph &sceneGraph, palette::Palette &palette,
										const LoadContext &loadctx) {
	core::ScopedPtr<io::SeekableReadStream> stream(archive->readStream(filename));
	if (!stream) {
		Log::error("Could not load file %s", filename.c_str());
		return false;
	}
	palette.minecraft();
	io::ZipReadStream zipStream(*stream);
	priv::NamedBinaryTagContext ctx;
	ctx.stream = &zipStream;
	const priv::NamedBinaryTag &schematic = priv::NamedBinaryTag::parse(ctx);
	if (!schematic.valid()) {
		Log::error("Could not find 'Schematic' tag");
		return false;
	}

	const core::String &extension = core::string::extractExtension(filename);
	if (extension == "nbt") {
		const int dataVersion = schematic.get("DataVersion").int32(-1);
		if (loadNbt(schematic, sceneGraph, palette, dataVersion)) {
			return true;
		}
	} else if (extension == "litematic") {
		return loadLitematic(schematic, sceneGraph, palette);
	}

	const int version = schematic.get("Version").int32(-1);
	Log::debug("Load schematic version %i", version);
	switch (version) {
	case 1:
	case 2:
		// WorldEdit legacy
		if (loadSponge1And2(schematic, sceneGraph, palette)) {
			return true;
		}
		// fall through
	case 3:
	default:
		if (loadSponge3(schematic, sceneGraph, palette, version)) {
			return true;
		}
	}
	schematic.print();
	return false;
}

bool SchematicFormat::loadSponge1And2(const priv::NamedBinaryTag &schematic, scenegraph::SceneGraph &sceneGraph,
									  palette::Palette &palette) {
	const priv::NamedBinaryTag &blockData = schematic.get("BlockData");
	if (blockData.valid() && blockData.type() == priv::TagType::BYTE_ARRAY) {
		return parseBlockData(schematic, sceneGraph, palette, blockData);
	}
	Log::error("Could not find valid 'BlockData' tags");
	return false;
}

bool SchematicFormat::loadSponge3(const priv::NamedBinaryTag &schematic, scenegraph::SceneGraph &sceneGraph,
								  palette::Palette &palette, int version) {
	const priv::NamedBinaryTag &blocks = schematic.get("Blocks");
	if (blocks.valid() && blocks.type() == priv::TagType::BYTE_ARRAY) {
		return parseBlocks(schematic, sceneGraph, palette, blocks, version);
	}
	Log::error("Could not find valid 'Blocks' tags");
	return false;
}

bool SchematicFormat::readLitematicBlockStates(const glm::ivec3 &size, int bits,
											   const priv::NamedBinaryTag &blockStates,
											   scenegraph::SceneGraphNode &node,
											   const core::Buffer<int> &mcpal) {
	const core::DynamicArray<int64_t> *data = blockStates.longArray();
	if (data == nullptr) {
		Log::error("Invalid BlockStates - expected long array");
		return false;
	}

	const uint64_t mask = (1 << bits) - 1;
	for (int y = 0; y < size.y; ++y) {
		for (int z = 0; z < size.z; ++z) {
			for (int x = 0; x < size.x; ++x) {
				const uint64_t index = size.x * size.z * y + size.x * z + x;
				const uint64_t startBit = index * bits;
				const uint64_t start = startBit / 64;
				const uint64_t rshiftVal = startBit & 63;
				const uint64_t end = startBit % 64 + bits;
				uint64_t id = 0;
				if (end <= 64 && start < data->size()) {
					id = (uint64_t)((*data)[start]) >> rshiftVal & mask;
				} else {
					if (start >= data->size() || start + 1 >= data->size()) {
						Log::error("Invalid BlockStates, out of bounds, start_state: %i, max size: %i, endnum: %i", (int)start,
								   (int)data->size(), (int)end);
						return false;
					}
					uint64_t move_num_2 = 64 - rshiftVal;
					id = (((uint64_t)(*data)[start]) >> rshiftVal | ((uint64_t)(*data)[start + 1]) << move_num_2) & mask;
				}
				if (id == 0) {
					continue;
				}
				const int colorIdx = mcpal[id];
				node.volume()->setVoxel(glm::ivec3(x, y, z), voxel::createVoxel(node.palette(), colorIdx));
			}
		}
	}
	return true;
}

bool SchematicFormat::loadLitematic(const priv::NamedBinaryTag &schematic, scenegraph::SceneGraph &sceneGraph,
									palette::Palette &palette) {
	const priv::NamedBinaryTag &versionNbt = schematic.get("Version");
	if (versionNbt.valid() && versionNbt.type() == priv::TagType::INT) {
		const int version = versionNbt.int32();
		Log::debug("version: %i", version);
		const priv::NamedBinaryTag &regions = schematic.get("Regions");
		if (!regions.compound()) {
			Log::error("Could not find valid 'Regions' compound tag");
			return false;
		}
		const priv::NBTCompound regionsCompound = *regions.compound();
		for (const auto &regionEntry : regionsCompound) {
			const priv::NamedBinaryTag &regionCompound = regionEntry->second;
			const core::String &name = regionEntry->first;
			const glm::ivec3 &pos = parsePosList(regionCompound, "Position");
			const glm::ivec3 &size = glm::abs(parsePosList(regionCompound, "Size"));
			const voxel::Region region({0, 0, 0}, size - 1);
			if (!region.isValid()) {
				Log::error("Invalid region mins: %i %i %i maxs: %i %i %i", pos.x, pos.y, pos.z, size.x, size.y, size.z);
				return false;
			}
			const priv::NamedBinaryTag &blockStatesPalette = regionCompound.get("BlockStatePalette");
			if (!blockStatesPalette.valid() || blockStatesPalette.type() != priv::TagType::LIST) {
				Log::error("Could not find 'BlockStatePalette'");
				return false;
			}

			const priv::NBTList &blockStatePaletteNbt = *blockStatesPalette.list();
			core::Buffer<int> mcpal;
			mcpal.resize(blockStatePaletteNbt.size());
			int paletteSize = 0;
			for (const auto & palNbt : blockStatePaletteNbt) {
				const priv::NamedBinaryTag &materialName = palNbt.get("Name");
				mcpal[paletteSize++] = findPaletteIndex(materialName.string()->c_str(), 1);
			}
			const int n = (int)blockStatePaletteNbt.size();
			int bits = 0;
			while (n > (1 << bits)) {
				++bits;
			}
			bits = core_max(bits, 2);

			const priv::NamedBinaryTag &blockStates = regionCompound.get("BlockStates");
			if (!blockStates.valid() || blockStates.type() != priv::TagType::LONG_ARRAY) {
				Log::error("Could not find 'BlockStates'");
				return false;
			}
			scenegraph::SceneGraphNode node;
			node.setPalette(palette);
			node.setName(name);
			node.setVolume(new voxel::RawVolume(region), true);
			if (!readLitematicBlockStates(size, bits, blockStates, node, mcpal)) {
				Log::error("Failed to read 'BlockStates'");
				return false;
			}
			if (sceneGraph.emplace(core::move(node)) == InvalidNodeId) {
				Log::error("Failed to add node to the scenegraph");
				return false;
			}
		}
		return true;
	}
	Log::error("Could not find valid 'Version' tag");
	return false;
}

bool SchematicFormat::loadNbt(const priv::NamedBinaryTag &schematic, scenegraph::SceneGraph &sceneGraph,
							  palette::Palette &palette, int dataVersion) {
	const priv::NamedBinaryTag &blocks = schematic.get("blocks");
	if (blocks.valid() && blocks.type() == priv::TagType::LIST) {
		const priv::NBTList &list = *blocks.list();
		glm::ivec3 mins((std::numeric_limits<int32_t>::max)() / 2);
		glm::ivec3 maxs((std::numeric_limits<int32_t>::min)() / 2);
		for (const priv::NamedBinaryTag &compound : list) {
			if (compound.type() != priv::TagType::COMPOUND) {
				Log::error("Unexpected nbt type: %i", (int)compound.type());
				return false;
			}
			const glm::ivec3 v = parsePosList(compound, "pos");
			mins = (glm::min)(mins, v);
			maxs = (glm::max)(maxs, v);
		}
		const voxel::Region region(mins, maxs);
		voxel::RawVolume *volume = new voxel::RawVolume(region);
		for (const priv::NamedBinaryTag &compound : list) {
			const int state = compound.get("state").int32();
			const glm::ivec3 v = parsePosList(compound, "pos");
			volume->setVoxel(v, voxel::createVoxel(palette, state));
		}
		scenegraph::SceneGraphNode node(scenegraph::SceneGraphNodeType::Model);
		const priv::NamedBinaryTag &author = schematic.get("author");
		if (author.valid() && author.type() == priv::TagType::STRING) {
			node.setProperty("Author", author.string());
		}
		node.setVolume(volume, true);
		node.setPalette(palette);
		int nodeId = sceneGraph.emplace(core::move(node));
		return nodeId != InvalidNodeId;
	}
	Log::error("Could not find valid 'blocks' tags");
	return false;
}

static glm::ivec3 voxelPosFromIndex(int width, int depth, int idx) {
	const int planeSize = width * depth;
	core_assert(planeSize != 0);
	const int y = idx / planeSize;
	const int offset = idx - (y * planeSize);
	const int z = offset / width;
	const int x = offset - z * width;
	return glm::ivec3(x, y, z);
}

bool SchematicFormat::parseBlockData(const priv::NamedBinaryTag &schematic, scenegraph::SceneGraph &sceneGraph,
									 palette::Palette &palette, const priv::NamedBinaryTag &blockData) {
	const core::DynamicArray<int8_t> *blocks = blockData.byteArray();
	if (blocks == nullptr) {
		Log::error("Invalid BlockData - expected byte array");
		return false;
	}
	core::Buffer<int> mcpal;
	const int paletteEntry = parsePalette(schematic, mcpal);

	const int16_t width = schematic.get("Width").int16();
	const int16_t height = schematic.get("Height").int16();
	const int16_t depth = schematic.get("Length").int16();

	if (width == 0 || depth == 0) {
		Log::error("Invalid width or length found");
		return false;
	}

	palette::PaletteLookup palLookup(palette);
	voxel::RawVolume *volume = new voxel::RawVolume(voxel::Region(0, 0, 0, width - 1, height - 1, depth - 1));
	SchematicIntReader reader(blocks);
	int index = 0;
	int32_t palIdx = 0;
	while (reader.readInt32(palIdx) != -1) {
		if (palIdx != 0) {
			uint8_t currentPalIdx;
			if (paletteEntry == 0) {
				currentPalIdx = palIdx;
			} else {
				currentPalIdx = mcpal[palIdx];
			}
			if (currentPalIdx != 0) {
				const glm::ivec3 &pos = voxelPosFromIndex(width, depth, index);
				volume->setVoxel(pos, voxel::createVoxel(palette, currentPalIdx));
			}
		}
		++index;
	}

	const int32_t x = schematic.get("x").int32();
	const int32_t y = schematic.get("y").int32();
	const int32_t z = schematic.get("z").int32();
	volume->translate(glm::ivec3(x, y, z));

	scenegraph::SceneGraphNode node(scenegraph::SceneGraphNodeType::Model);
	node.setVolume(volume, true);
	node.setPalette(palLookup.palette());
	int nodeId = sceneGraph.emplace(core::move(node));
	if (nodeId == -1) {
		return false;
	}
	parseMetadata(schematic, sceneGraph, sceneGraph.node(nodeId));
	return true;
}

bool SchematicFormat::parseBlocks(const priv::NamedBinaryTag &schematic, scenegraph::SceneGraph &sceneGraph,
								  palette::Palette &palette, const priv::NamedBinaryTag &blocks, int version) {
	core::Buffer<int> mcpal;
	const int paletteEntry = parsePalette(schematic, mcpal);

	const int16_t width = schematic.get("Width").int16();
	const int16_t height = schematic.get("Height").int16();
	const int16_t depth = schematic.get("Length").int16();

	// TODO: Support for WorldEdit's AddBlocks is missing
	// * https://github.com/EngineHub/WorldEdit/blob/master/worldedit-core/src/main/java/com/sk89q/worldedit/extent/clipboard/io/MCEditSchematicReader.java#L171
	// * https://github.com/mcedit/mcedit2/blob/master/src/mceditlib/schematic.py#L143
	// * https://github.com/Lunatrius/Schematica/blob/master/src/main/java/com/github/lunatrius/schematica/world/schematic/SchematicAlpha.java

	palette::PaletteLookup palLookup(palette);
	voxel::RawVolume *volume = new voxel::RawVolume(voxel::Region(0, 0, 0, width - 1, height - 1, depth - 1));
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			for (int z = 0; z < depth; ++z) {
				const int idx = (y * depth + z) * width + x;
				const uint8_t palIdx = (*blocks.byteArray())[idx];
				if (palIdx != 0u) {
					uint8_t currentPalIdx;
					if (paletteEntry == 0 || palIdx > paletteEntry) {
						currentPalIdx = palIdx;
					} else {
						currentPalIdx = mcpal[palIdx];
					}
					volume->setVoxel(x, y, z, voxel::createVoxel(palette, currentPalIdx));
				}
			}
		}
	}

	const int32_t x = schematic.get("x").int32();
	const int32_t y = schematic.get("y").int32();
	const int32_t z = schematic.get("z").int32();
	volume->translate(glm::ivec3(x, y, z));

	scenegraph::SceneGraphNode node(scenegraph::SceneGraphNodeType::Model);
	node.setVolume(volume, true);
	node.setPalette(palLookup.palette());
	int nodeId = sceneGraph.emplace(core::move(node));
	if (nodeId == -1) {
		return false;
	}
	parseMetadata(schematic, sceneGraph, sceneGraph.node(nodeId));
	return true;
}

int SchematicFormat::parsePalette(const priv::NamedBinaryTag &schematic, core::Buffer<int> &mcpal) const {
	const priv::NamedBinaryTag &blockIds = schematic.get("BlockIDs"); // MCEdit2
	if (blockIds.valid()) {
		mcpal.resize(palette::PaletteMaxColors);
		int paletteEntry = 0;
		const int blockCnt = (int)blockIds.compound()->size();
		for (int i = 0; i < blockCnt; ++i) {
			const priv::NamedBinaryTag &nbt = blockIds.get(core::String::format("%i", i));
			const core::String *value = nbt.string();
			if (value == nullptr) {
				Log::warn("Empty string in BlockIDs for %i", i);
				continue;
			}
			// map to stone on default
			mcpal[i] = findPaletteIndex(*value, 1);
			++paletteEntry;
		}
		return paletteEntry;
	}
	const int paletteMax = schematic.get("PaletteMax").int32(-1); // WorldEdit
	if (paletteMax != -1) {
		const priv::NamedBinaryTag &palette = schematic.get("Palette");
		if (palette.valid() && palette.type() == priv::TagType::COMPOUND) {
			if ((int)palette.compound()->size() != paletteMax) {
				return -1;
			}
			mcpal.resize(paletteMax);
			int paletteEntry = 0;
			for (const auto &c : *palette.compound()) {
				core::String key = c->key;
				const int palIdx = c->second.int32(-1);
				if (palIdx == -1) {
					Log::warn("Failed to get int value for %s", key.c_str());
					continue;
				}
				// map to stone on default
				mcpal[palIdx] = findPaletteIndex(key, 1);
				++paletteEntry;
			}
			return paletteEntry;
		}
	}
	return -1;
}

void SchematicFormat::parseMetadata(const priv::NamedBinaryTag &schematic, scenegraph::SceneGraph &sceneGraph,
									scenegraph::SceneGraphNode &node) {
	const priv::NamedBinaryTag &metadata = schematic.get("Metadata");
	if (metadata.valid()) {
		if (const core::String *str = metadata.get("Name").string()) {
			node.setName(*str);
		}
		if (const core::String *str = metadata.get("Author").string()) {
			node.setProperty("Author", *str);
		}
	}
	const int version = schematic.get("Version").int32(-1);
	if (version != -1) {
		node.setProperty("Version", core::string::toString(version));
	}
	core_assert_msg(node.id() != -1, "The node should already be part of the scene graph");
	for (const auto &e : *schematic.compound()) {
		addMetadata_r(e->key, e->value, sceneGraph, node);
	}
}

void SchematicFormat::addMetadata_r(const core::String &key, const priv::NamedBinaryTag &nbt,
									scenegraph::SceneGraph &sceneGraph, scenegraph::SceneGraphNode &node) {
	switch (nbt.type()) {
	case priv::TagType::COMPOUND: {
		scenegraph::SceneGraphNode compoundNode(scenegraph::SceneGraphNodeType::Group);
		compoundNode.setName(key);
		int nodeId = sceneGraph.emplace(core::move(compoundNode), node.id());
		for (const auto &e : *nbt.compound()) {
			addMetadata_r(e->key, e->value, sceneGraph, sceneGraph.node(nodeId));
		}
		break;
	}
	case priv::TagType::END:
	case priv::TagType::BYTE:
		node.setProperty(key, core::string::toString(nbt.int8()));
		break;
	case priv::TagType::SHORT:
		node.setProperty(key, core::string::toString(nbt.int16()));
		break;
	case priv::TagType::INT:
		node.setProperty(key, core::string::toString(nbt.int32()));
		break;
	case priv::TagType::LONG:
		node.setProperty(key, core::string::toString(nbt.int64()));
		break;
	case priv::TagType::FLOAT:
		node.setProperty(key, core::string::toString(nbt.float32()));
		break;
	case priv::TagType::DOUBLE:
		node.setProperty(key, core::string::toString(nbt.float64()));
		break;
	case priv::TagType::STRING:
		node.setProperty(key, nbt.string());
		break;
	case priv::TagType::LIST: {
		const priv::NBTList &list = *nbt.list();
		scenegraph::SceneGraphNode listNode(scenegraph::SceneGraphNodeType::Group);
		listNode.setName(core::string::format("%s: %i", key.c_str(), (int)list.size()));
		int nodeId = sceneGraph.emplace(core::move(listNode), node.id());
		for (const priv::NamedBinaryTag &e : list) {
			addMetadata_r(key, e, sceneGraph, sceneGraph.node(nodeId));
		}
		break;
	}
	case priv::TagType::BYTE_ARRAY:
		node.setProperty(key, "Byte Array");
		break;
	case priv::TagType::INT_ARRAY:
		node.setProperty(key, "Int Array");
		break;
	case priv::TagType::LONG_ARRAY:
		node.setProperty(key, "Long Array");
		break;
	case priv::TagType::MAX:
		break;
	}
}

bool SchematicFormat::saveGroups(const scenegraph::SceneGraph &sceneGraph, const core::String &filename,
								 const io::ArchivePtr &archive, const SaveContext &ctx) {
	core::ScopedPtr<io::SeekableWriteStream> stream(archive->writeStream(filename));
	if (!stream) {
		Log::error("Could not open file %s", filename.c_str());
		return false;
	}
	// save as sponge-3
	const scenegraph::SceneGraph::MergedVolumePalette &merged = sceneGraph.merge();
	if (merged.first == nullptr) {
		Log::error("Failed to merge volumes");
		return false;
	}
	core::ScopedPtr<voxel::RawVolume> scopedPtr(merged.first);
	const voxel::Region &region = merged.first->region();
	const glm::ivec3 &size = region.getDimensionsInVoxels();
	const glm::ivec3 &mins = region.getLowerCorner();

	io::ZipWriteStream zipStream(*stream);

	priv::NBTCompound compound;
	compound.put("Width", (int16_t)size.x);
	compound.put("Height", (int16_t)size.y);
	compound.put("Length", (int16_t)size.z);
	compound.put("x", (int32_t)mins.x);
	compound.put("y", (int32_t)mins.y);
	compound.put("z", (int32_t)mins.z);
	compound.put("Materials", priv::NamedBinaryTag("Alpha"));
	compound.put("Version", 3);
	// TODO: palette
	{
		core::DynamicArray<int8_t> blocks;
		blocks.resize((size_t)size.x * (size_t)size.y * (size_t)size.z);

		for (int x = 0; x < size.x; ++x) {
			for (int y = 0; y < size.y; ++y) {
				for (int z = 0; z < size.z; ++z) {
					const int idx = (y * size.z + z) * size.x + x;
					const voxel::Voxel &voxel = merged.first->voxel(mins.x + x, mins.y + y, mins.z + z);
					if (voxel::isAir(voxel.getMaterial())) {
						blocks[idx] = 0;
					} else {
						const uint8_t currentPalIdx = voxel.getColor();
						blocks[idx] = (int8_t)currentPalIdx;
					}
				}
			}
		}
		compound.put("Blocks", priv::NamedBinaryTag(core::move(blocks)));
	}
	const priv::NamedBinaryTag tag(core::move(compound));
	return priv::NamedBinaryTag::write(tag, "Schematic", zipStream);
}

} // namespace voxelformat
