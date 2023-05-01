/**
 * @file
 */

#include "VMaxFormat.h"
#include "core/FourCC.h"
#include "core/Log.h"
#include "external/json.hpp"
#include "image/Image.h"
#include "io/BufferedReadWriteStream.h"
#include "io/LZFSEReadStream.h"
#include "io/MemoryReadStream.h"
#include "io/ZipArchive.h"
#include "private/BinaryPList.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/SceneGraphNode.h"
#include "voxel/Palette.h"
#include "voxel/RawVolume.h"
#include "voxel/RawVolumeWrapper.h"
#include "voxel/Region.h"
#include "voxelformat/Format.h"
#include "voxel/Morton.h"
#include <glm/common.hpp>

namespace voxelformat {

#define jsonVec(json, name, obj)                                                                                       \
	if ((json).find(#name) == (json).end() || !(json)[#name].is_array()) {                                             \
		const std::string dump = (json).dump();                                                                        \
		Log::debug("Failed to parse json array " #name ": %s", dump.c_str());                                          \
	} else {                                                                                                           \
		for (int i = 0; i < (obj).name.length(); ++i) {                                                                \
			(obj).name[i] = (json)[#name][i].get<float>();                                                             \
		}                                                                                                              \
	}

#define jsonInt(json, name, obj)                                                                                       \
	if ((json).find(#name) == (json).end() || !(json)[#name].is_number_integer()) {                                    \
		const std::string dump = (json).dump();                                                                        \
		Log::debug("Failed to parse json integer " #name ": %s", dump.c_str());                                        \
	} else {                                                                                                           \
		(obj).name = (json)[#name].get<int>();                                                                         \
	}

#define jsonFloat(json, name, obj)                                                                                     \
	if ((json).find(#name) == (json).end() || !(json)[#name].is_number_float()) {                                      \
		const std::string dump = (json).dump();                                                                        \
		Log::debug("Failed to parse json float " #name ": %s", dump.c_str());                                          \
	} else {                                                                                                           \
		(obj).name = (json)[#name].get<float>();                                                                       \
	}

#define jsonBool(json, name, obj)                                                                                      \
	if ((json).find(#name) == (json).end() || !(json)[#name].is_boolean()) {                                           \
		const std::string dump = (json).dump();                                                                        \
		Log::debug("Failed to parse json bool " #name ": %s", dump.c_str());                                           \
	} else {                                                                                                           \
		(obj).name = (json)[#name].get<bool>();                                                                        \
	}

#define jsonString(json, name, obj)                                                                                    \
	if ((json).find(#name) == (json).end() || !(json)[#name].is_string()) {                                            \
		const std::string dump = (json).dump();                                                                        \
		Log::debug("Failed to parse json string " #name ": %s", dump.c_str());                                         \
	} else {                                                                                                           \
		(obj).name = (json)[#name].get<std::string>().c_str();                                                         \
	}

#define wrap(action) \
	if ((action) == -1) { \
		Log::error("Error: Failed to execute " CORE_STRINGIFY(action) " (line %i)", (int)__LINE__); \
		return false; \
	}

bool VMaxFormat::loadSceneJson(io::ZipArchive &archive, VMaxScene &scene) const {
	io::BufferedReadWriteStream contentsStream;
	if (!archive.load("scene.json", contentsStream)) {
		Log::error("Failed to load scene.json");
		return false;
	}

	contentsStream.seek(0);

	std::string jsonStr((const char *)contentsStream.getBuffer(), contentsStream.size());
	nlohmann::json json = nlohmann::json::parse(jsonStr);
	if (json.is_null()) {
		Log::error("Failed to parse the json");
		return false;
	}

	jsonString(json, af, scene);
	jsonFloat(json, aint, scene);
	jsonFloat(json, eint, scene);
	jsonFloat(json, outlinesz, scene);
	jsonFloat(json, sat, scene);
	jsonFloat(json, shadowint, scene);
	jsonFloat(json, temp, scene);
	jsonFloat(json, cont, scene);
	jsonFloat(json, tint, scene);
	jsonString(json, background, scene);
	jsonString(json, lcolor, scene);
	jsonFloat(json, bloombrad, scene);
	jsonFloat(json, bloomint, scene);
	jsonFloat(json, bloomthr, scene);
	jsonInt(json, v, scene);
	jsonFloat(json, outlineint, scene);
	jsonBool(json, nrn, scene);
	jsonBool(json, ssr, scene);
	jsonFloat(json, lint, scene);

	auto objects = json.find("objects");
	if (objects == json.end() || !objects->is_array()) {
		Log::error("Failed to parse the scene json - expected an array of objects");
		return false;
	}
	for (const auto &obj : objects.value()) {
		VMaxObject o;
		jsonBool(obj, s, o);
		jsonBool(obj, h, o);
		jsonString(obj, n, o);
		jsonString(obj, data, o);
		jsonString(obj, pal, o);
		jsonString(obj, pid, o);
		jsonString(obj, hist, o);
		jsonString(obj, id, o);
		jsonString(obj, t_al, o);
		jsonString(obj, t_pa, o);
		jsonString(obj, t_po, o);
		jsonString(obj, t_pf, o);
		jsonVec(obj, ind, o);
		jsonVec(obj, e_c, o);
		jsonVec(obj, e_mi, o);
		jsonVec(obj, e_ma, o);
		jsonVec(obj, t_p, o);
		jsonVec(obj, t_s, o);
		jsonVec(obj, t_r, o);
		o.e_c = glm::ceil(o.e_c);
		o.e_mi = glm::ceil(o.e_mi);
		o.e_ma = glm::ceil(o.e_ma);
		scene.objects.push_back(o);
	}

	return true;
}

bool VMaxFormat::loadGroupsPalette(const core::String &filename, io::SeekableReadStream &stream,
								   scenegraph::SceneGraph &sceneGraph, voxel::Palette &palette,
								   const LoadContext &ctx) {
	io::ZipArchive archive;
	if (!archive.open(&stream)) {
		Log::error("Failed to open zip archive %s", filename.c_str());
		return false;
	}

	VMaxScene scene;
	if (!loadSceneJson(archive, scene)) {
		return false;
	}

	// layers are in own contents files. They start at contents, contents1, contents2, ... and so on. The same
	// is true for palettes
	for (const auto &obj : scene.objects) {
		if (!loadObject(filename, archive, sceneGraph, ctx, obj)) {
			Log::error("Failed to load object %s", obj.n.c_str());
			return false;
		}
	}
	return true;
}

bool VMaxFormat::loadVolume(const core::String &filename, io::ZipArchive &archive, const LoadContext &ctx,
							const VMaxObject &obj, voxel::RawVolume *v) const {
	io::BufferedReadWriteStream data;
	if (!archive.load(obj.data, data)) {
		Log::error("Failed to load %s", obj.data.c_str());
		return false;
	}
	if (data.seek(0) == -1) {
		Log::error("Failed to seek to the beginning of the sub stream");
		return false;
	}
	io::LZFSEReadStream stream(data);

	// io::filesystem()->write(filename + ".plist", stream);
	// stream.seek(0);

	priv::BinaryPList plist = priv::BinaryPList::parse(stream);
	if (!plist.isDict()) {
		Log::error("Expected a bplist dict");
		return false;
	}

	const priv::PListDict &dict = plist.asDict();
	auto snapshots = dict.find("snapshots");
	if (snapshots == dict.end()) {
		Log::error("No 'snapshots' node found in bplist");
		return false;
	}
	if (!snapshots->value.isArray()) {
		Log::error("Node 'snapshots' has unexpected type");
		return false;
	}
	const priv::PListArray &snapshotsArray = snapshots->value.asArray();
	if (snapshotsArray.empty()) {
		Log::error("Node 'snapshots' is empty");
		return false;
	}

	for (size_t i = 0; i < snapshotsArray.size(); ++i) {
		const priv::BinaryPList &snapshot = snapshotsArray[i].getDictEntry("s");
		if (snapshot.empty()) {
			Log::error("Node 'snapshots' child %i doesn't contain node 's'", (int)i);
			return false;
		}

		// const priv::BinaryPList &deselectedLayerColorUsage = snapshot.getDictEntry("dlc");
		const priv::BinaryPList &data = snapshot.getDictEntry("ds");
		// const priv::BinaryPList &layerColorUsage = snapshot.getDictEntry("lc");
		// const priv::BinaryPList &stats = snapshot.getDictEntry("st");
		const priv::BinaryPList &identifier = snapshot.getDictEntry("id");
		const priv::BinaryPList &identifierC = identifier.getDictEntry("c");
		const priv::BinaryPList &identifierS = identifier.getDictEntry("s");
		const priv::BinaryPList &identifierT = identifier.getDictEntry("t");

		uint64_t mortonChunkIdx = 0, idTimeline = 0;
		SnapshotType type = SnapshotType::UndoRestore;
		if (identifierC.isInt()) {
			mortonChunkIdx = identifierC.asInt();
		}
		if (identifierS.isInt()) {
			idTimeline = identifierS.asInt();
		}
		if (identifierT.isInt()) {
			type = (SnapshotType)identifierT.asUInt8();
		}

		// max volume size 256x256x256 and each chunk is 32x32x32 - to the max amount of chunks are 8x8x8 (512)
		constexpr uint64_t MaxVolumeSize = 256u;
		constexpr uint64_t MaxChunkSize = 32u; // TODO: this is theoretically variable - but we don't support it yet
		constexpr uint64_t MaxVolumeChunks = MaxVolumeSize / MaxChunkSize;
		constexpr uint64_t MaxChunks = MaxVolumeChunks * MaxVolumeChunks * MaxVolumeChunks;
		if (mortonChunkIdx > MaxChunks) {
			Log::error("identifier: c(%i) is out of range", (int)mortonChunkIdx);
			return false;
		}

		Log::debug("identifier: c(%i), s(%i), t(%i)", (int)mortonChunkIdx, (int)idTimeline, (int)type);

		const size_t dsSize = data.size();
		if (dsSize == 0u) {
			Log::error("Node 'ds' is empty");
			return false;
		}

		io::MemoryReadStream dsStream(data.asData().data(), dsSize);
		Log::debug("Found voxel data with size %i", (int)dsStream.size());

		// search the chunk world position by getting the morton index for the snapshot id
		for (int x = 0; x < (int)MaxVolumeSize; ++x) {
			for (int y = 0; y < (int)MaxVolumeSize; ++y) {
				for (int z = 0; z < (int)MaxVolumeSize; ++z) {
					if (mortonChunkIdx != voxel::mortonIndex(x, y, z)) {
						// chunk world position doesn't match the snapshot id
						continue;
					}
					Log::debug("Found chunk pos at %i:%i:%i", x, y, z);

					const glm::ivec3 mins(x * (int)MaxChunkSize, y * (int)MaxChunkSize, z * (int)MaxChunkSize);
					const glm::ivec3 maxs = mins + (int)(MaxChunkSize - 1);
					const voxel::Region region(mins, maxs);
					voxel::RawVolumeWrapper wrapper(v, region);
					// read the voxels from the stream
					uint32_t mortonIdx = 0u;
					while (dsStream.remaining() > 0) {
						// there are only 8 materials used for now 0-7 and 8 selected versions for them 8-15,
						// with option to add more in the future up to 128
						uint8_t extendedLayerInfo;
						// palette index 0 means air
						uint8_t palIdx;
						wrap(dsStream.readUInt8(extendedLayerInfo))
						wrap(dsStream.readUInt8(palIdx))
						// the voxels are stored in morton order - use the index to find the voxel position
						++mortonIdx;
						if (palIdx == 0) {
							continue;
						}
						// this loop is using the morton index to get the voxel position
						for (int vx = 0; vx < (int)MaxChunkSize; ++vx) {
							for (int vy = 0; vy < (int)MaxChunkSize; ++vy) {
								for (int vz = 0; vz < (int)MaxChunkSize; ++vz) {
									const uint32_t idx = voxel::mortonIndex(vx, vy, vz);
									if (idx != mortonIdx) {
										continue;
									}
									// TODO: maybe y and z are swapped?
									wrapper.setVoxel(mins.x + vx, mins.y + vy, mins.z + vz, voxel::createVoxel(voxel::VoxelType::Generic, palIdx));
								}
							}
						}
					}
					break;
				}
			}
		}
	}

	Log::error("Not yet supported to load the voxel data");
	return false;
}

bool VMaxFormat::loadObject(const core::String &filename, io::ZipArchive &archive, scenegraph::SceneGraph &sceneGraph,
							const LoadContext &ctx, const VMaxObject &obj) const {
	voxel::Palette palette;
	if (!loadPalette(archive, obj.pal, palette, ctx)) {
		return false;
	}

	const glm::vec3 mins = obj.e_c + obj.e_mi;
	const glm::vec3 maxs = obj.e_c + obj.e_ma;
	const voxel::Region region(glm::ivec3(mins), glm::ivec3(maxs) - 1);
	if (!region.isValid()) {
		Log::error("Invalid region for object '%s': %s", obj.n.c_str(), region.toString().c_str());
		return false;
	}
	voxel::RawVolume *v = new voxel::RawVolume(region);

	loadVolume(filename, archive, ctx, obj, v);

	scenegraph::SceneGraphTransform transform;
	transform.setWorldScale(obj.t_s);
	transform.setWorldTranslation(obj.t_p);
	transform.setWorldOrientation(glm::quat(obj.t_r.w, obj.t_r.x, obj.t_r.y, obj.t_r.z));

	scenegraph::SceneGraphNode node(scenegraph::SceneGraphNodeType::Model);
	node.setName(obj.n);
	node.setVisible(!obj.h);
	node.setPalette(palette);
	node.setVolume(v, true);
	scenegraph::KeyFrameIndex keyFrameIdx = 0;
	node.setTransform(keyFrameIdx, transform);
	return sceneGraph.emplace(core::move(node)) != InvalidNodeId;
}

image::ImagePtr VMaxFormat::loadScreenshot(const core::String &filename, io::SeekableReadStream &stream,
										   const LoadContext &ctx) {
	io::ZipArchive archive;
	if (!archive.open(&stream)) {
		Log::error("Failed to open zip archive %s", filename.c_str());
		return image::ImagePtr();
	}

	io::BufferedReadWriteStream contentsStream;
	if (!archive.load("QuickLook/Thumbnail.png", contentsStream)) {
		Log::error("Failed to load QuickLook/Thumbnail.png from %s", filename.c_str());
		return image::ImagePtr();
	}

	if (contentsStream.seek(0) == -1) {
		Log::error("Failed to seek to the beginning of the sub stream for %s", filename.c_str());
		return 0u;
	}
	return image::loadImage("Thumbnail.png", contentsStream);
}

bool VMaxFormat::loadPalette(io::ZipArchive &archive, const core::String &paletteName, voxel::Palette &palette,
							 const LoadContext &ctx) const {
	io::BufferedReadWriteStream contentsStream;
	if (!archive.load(paletteName, contentsStream)) {
		Log::error("Failed to load %s", paletteName.c_str());
		return false;
	}

	if (contentsStream.seek(0) == -1) {
		Log::error("Failed to seek to the beginning of the sub stream for the palette %s", paletteName.c_str());
		return false;
	}

	const image::ImagePtr &img = image::loadImage(paletteName, contentsStream);
	if (!img->isLoaded()) {
		Log::error("Failed to load image %s", paletteName.c_str());
		return false;
	}
	if (!palette.load(img)) {
		Log::error("Failed to load palette from image %s", paletteName.c_str());
		return false;
	}
	return true;
}

size_t VMaxFormat::loadPalette(const core::String &filename, io::SeekableReadStream &stream, voxel::Palette &palette,
							   const LoadContext &ctx) {
	io::ZipArchive archive;
	if (!archive.open(&stream)) {
		Log::error("Failed to open zip archive %s", filename.c_str());
		return 0u;
	}

	if (!loadPalette(archive, "palette.png", palette, ctx)) {
		return 0u;
	}
	return palette.colorCount();
}

#undef jsonVec
#undef jsonInt
#undef jsonFloat
#undef jsonBool
#undef jsonString
#undef wrap

} // namespace voxelformat
