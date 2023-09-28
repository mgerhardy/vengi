/**
 * @file
 */

#pragma once

#include "voxelformat/Format.h"
#include "core/collection/Buffer.h"
#include "core/collection/DynamicArray.h"
#include "core/collection/DynamicMap.h"
#include "voxel/Palette.h"

namespace io {
class ZipReadStream;
}

namespace voxelformat {

namespace priv {
class NamedBinaryTag;
using NBTCompound = core::DynamicMap<core::String, NamedBinaryTag, 11, core::StringHash>;
using NBTList = core::DynamicArray<NamedBinaryTag>;
} // namespace priv

/**
 * A minecraft chunk contains the terrain and entity information about a grid of the size 16x256x16
 *
 * A section is 16x16x16 and a chunk contains max 16 sections. Section 0 is the bottom, section 15 is the top
 *
 * @note This is stored in NBT format
 *
 * older version:
 * @code
 * root tag (compound)
 *   \-- DataVersion - version of the nbt chunk
 *   \-- Level - chunk data (compound)
 *     \-- xPos - x pos in chunk relative to the origin (not the region)
 *     \-- yPos - y pos in chunk relative to the origin (not the region)
 *     \-- Sections (list)
 *       \-- section (compound)
 *         \-- Y: Range 0 to 15 (bottom to top) - if empty, section is empty
 *         \-- Palette
 *         \-- BlockLight - 2048 bytes
 *         \-- BlockStates
 *         \-- SkyLight
 * @endcode
 * newer version
 * the block_states are under a sections compound
 *
 * @code
 * byte Nibble4(byte[] arr, int index) {
 *   return index%2 == 0 ? arr[index/2]&0x0F : (arr[index/2]>>4)&0x0F;
 * }
 * int BlockPos = y*16*16 + z*16 + x;
 * compound Block = Palette[change_array_element_size(BlockStates,Log2(length(Palette)))[BlockPos]]
 * string BlockName = Block.Name;
 * compound BlockState = Block.Properties;
 * byte Blocklight = Nibble4(BlockLight, BlockPos);
 * byte Skylight = Nibble4(SkyLight, BlockPos);
 * @endcode
 *
 * @note https://github.com/Voxtric/Minecraft-Level-Ripper/blob/master/WorldConverterV2/Processor.cs
 * @note https://minecraft.wiki/w/Region_file_format
 * @note https://minecraft.wiki/w/Chunk_format
 * @note https://github.com/UnknownShadow200/ClassiCube/blob/master/src/Formats.c
 *
 * @ingroup Formats
 */
class MCRFormat : public PaletteFormat {
public:
	static constexpr int SECTOR_BYTES = 4096;
	static constexpr int SECTOR_INTS = SECTOR_BYTES / 4;

private:
	static constexpr int VERSION_GZIP = 1;
	static constexpr int VERSION_DEFLATE = 2;
	static constexpr int MAX_SIZE = 16;

	struct Offsets {
		uint32_t offset;
		uint8_t sectorCount;
	} _offsets[SECTOR_INTS];

	struct MinecraftSectionPalette {
		core::Buffer<uint8_t> pal;
		uint32_t numBits = 0u;
		voxel::Palette mcpal;
	};

	using SectionVolumes = core::DynamicArray<voxel::RawVolume *>;

	voxel::RawVolume *error(SectionVolumes &volumes);
	voxel::RawVolume *finalize(SectionVolumes &volumes, int xPos, int zPos);

	static int getVoxel(int dataVersion, const priv::NamedBinaryTag &data, const glm::ivec3 &pos);

	// shared across versions
	bool parsePaletteList(int dataVersion, const priv::NamedBinaryTag &palette, MinecraftSectionPalette &sectionPal);
	bool parseBlockStates(int dataVersion, const voxel::Palette &palette, const priv::NamedBinaryTag &data,
						  SectionVolumes &volumes, int sectionY, const MinecraftSectionPalette &secPal);

	// new version (>= 2844)
	voxel::RawVolume *parseSections(int dataVersion, const priv::NamedBinaryTag &root, int sector,
									const voxel::Palette &palette);

	// old version (< 2844)
	voxel::RawVolume *parseLevelCompound(int dataVersion, const priv::NamedBinaryTag &root, int sector,
										 const voxel::Palette &palette);

	bool readCompressedNBT(scenegraph::SceneGraph &sceneGraph, io::SeekableReadStream &stream, int sector,
						   const voxel::Palette &palette);
	bool loadMinecraftRegion(scenegraph::SceneGraph &sceneGraph, io::SeekableReadStream &stream,
							 const voxel::Palette &palette);

	bool saveSections(const scenegraph::SceneGraph &sceneGraph, priv::NBTList &sections, int sector);
	bool saveCompressedNBT(const scenegraph::SceneGraph &sceneGraph, io::SeekableWriteStream &stream, int sector);
	bool saveMinecraftRegion(const scenegraph::SceneGraph &sceneGraph, io::SeekableWriteStream &stream);

protected:
	bool loadGroupsPalette(const core::String &filename, io::SeekableReadStream &stream,
						   scenegraph::SceneGraph &sceneGraph, voxel::Palette &palette,
						   const LoadContext &ctx) override;
	bool saveGroups(const scenegraph::SceneGraph &sceneGraph, const core::String &filename,
					io::SeekableWriteStream &stream, const SaveContext &ctx) override;
};

} // namespace voxelformat
