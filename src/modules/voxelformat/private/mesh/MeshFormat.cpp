/**
 * @file
 */

#include "MeshFormat.h"
#include "app/App.h"
#include "core/Algorithm.h"
#include "core/Color.h"
#include "core/GLM.h"
#include "core/GameConfig.h"
#include "core/Log.h"
#include "core/RGBA.h"
#include "core/StringUtil.h"
#include "core/Var.h"
#include "core/collection/DynamicArray.h"
#include "core/collection/DynamicMap.h"
#include "core/collection/Map.h"
#include "core/concurrent/Lock.h"
#include "core/concurrent/ThreadPool.h"
#include "io/FormatDescription.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/SceneGraphNode.h"
#include "voxel/ChunkMesh.h"
#include "voxel/SurfaceExtractor.h"
#include "voxel/MaterialColor.h"
#include "voxel/Mesh.h"
#include "voxel/RawVolume.h"
#include "voxel/RawVolumeWrapper.h"
#include "Tri.h"
#include "voxelutil/VoxelUtil.h"
#include <SDL_timer.h>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/epsilon.hpp>

namespace voxelformat {

MeshFormat::MeshFormat() {
	_flattenFactor = core::Var::getSafe(cfg::VoxformatRGBFlattenFactor)->intVal();
}

MeshFormat::MeshExt *MeshFormat::getParent(const scenegraph::SceneGraph &sceneGraph, MeshFormat::Meshes &meshes,
										   int nodeId) {
	if (!sceneGraph.hasNode(nodeId)) {
		return nullptr;
	}
	const int parent = sceneGraph.node(nodeId).parent();
	for (MeshExt &me : meshes) {
		if (me.nodeId == parent) {
			return &me;
		}
	}
	return nullptr;
}

glm::vec3 MeshFormat::getScale() {
	const float scale = core::Var::getSafe(cfg::VoxformatScale)->floatVal();

	float scaleX = core::Var::getSafe(cfg::VoxformatScaleX)->floatVal();
	float scaleY = core::Var::getSafe(cfg::VoxformatScaleY)->floatVal();
	float scaleZ = core::Var::getSafe(cfg::VoxformatScaleZ)->floatVal();

	scaleX = glm::epsilonNotEqual(scaleX, 1.0f, glm::epsilon<float>()) ? scaleX : scale;
	scaleY = glm::epsilonNotEqual(scaleY, 1.0f, glm::epsilon<float>()) ? scaleY : scale;
	scaleZ = glm::epsilonNotEqual(scaleZ, 1.0f, glm::epsilon<float>()) ? scaleZ : scale;
	Log::debug("scale: %f:%f:%f", scaleX, scaleY, scaleZ);
	return {scaleX, scaleY, scaleZ};
}

void MeshFormat::subdivideTri(const Tri &tri, TriCollection &tinyTris) {
	if (stopExecution()) {
		return;
	}
	const glm::vec3 &mins = tri.mins();
	const glm::vec3 &maxs = tri.maxs();
	const glm::vec3 size = maxs - mins;
	if (glm::any(glm::greaterThan(size, glm::vec3(1.0f)))) {
		Tri out[4];
		tri.subdivide(out);
		for (int i = 0; i < lengthof(out); ++i) {
			subdivideTri(out[i], tinyTris);
		}
		return;
	}
	tinyTris.push_back(tri);
}

core::RGBA MeshFormat::PosSampling::avgColor(uint8_t flattenFactor) const {
	if (entries.size() == 1) {
		return core::Color::flattenRGB(entries[0].color.r, entries[0].color.g, entries[0].color.b, entries[0].color.a,
									   flattenFactor);
	}
	float sumArea = 0.0f;
	for (const PosSamplingEntry &pe : entries) {
		sumArea += pe.area;
	}
	core::RGBA color(0, 0, 0, 255);
	if (sumArea <= 0.0f) {
		return color;
	}
	for (const PosSamplingEntry &pe : entries) {
		color = core::RGBA::mix(color, pe.color, pe.area / sumArea);
	}
	return core::Color::flattenRGB(color.r, color.g, color.b, color.a, flattenFactor);
}

glm::vec2 MeshFormat::paletteUV(int colorIndex) {
	// 1 x 256 is the texture format that we are using for our palette
	const glm::vec2 &uv = image::Image::uv(colorIndex, 0, voxel::PaletteMaxColors, 1);
	return uv;
}

void MeshFormat::transformTris(const TriCollection &subdivided, PosMap &posMap) {
	Log::debug("subdivided into %i triangles", (int)subdivided.size());
	for (const Tri &tri : subdivided) {
		if (stopExecution()) {
			return;
		}
		const float area = tri.area();
		const core::RGBA rgba = tri.centerColor();
		const glm::ivec3 p(glm::round(tri.center()));
		auto iter = posMap.find(p);
		if (iter == posMap.end()) {
			posMap.emplace(p, {area, rgba});
		} else if (iter->value.entries.size() < 4 && iter->value.entries[0].color != rgba) {
			PosSampling &pos = iter->value;
			pos.entries.emplace_back(area, rgba);
		}
	}
}

void MeshFormat::transformTrisAxisAligned(const TriCollection &tris, PosMap &posMap) {
	Log::debug("%i triangles", (int)tris.size());
	for (const Tri &tri : tris) {
		if (stopExecution()) {
			return;
		}
		const core::RGBA rgba = tri.centerColor();
		const float area = tri.area();
		const glm::vec3 &normal = glm::normalize(tri.normal());
		const glm::ivec3 sideDelta(normal.x <= 0 ? 0 : -1, normal.y <= 0 ? 0 : -1, normal.z <= 0 ? 0 : -1);
		const glm::ivec3 mins = tri.roundedMins();
		const glm::ivec3 maxs = tri.roundedMaxs() + glm::ivec3(glm::round(glm::abs(normal)));
		Log::debug("mins: %i:%i:%i", mins.x, mins.y, mins.z);
		Log::debug("maxs: %i:%i:%i", maxs.x, maxs.y, maxs.z);
		Log::debug("normal: %f:%f:%f", normal.x, normal.y, normal.z);
		Log::debug("sideDelta: %i:%i:%i", sideDelta.x, sideDelta.y, sideDelta.z);

		for (int x = mins.x; x < maxs.x; x++) {
			for (int y = mins.y; y < maxs.y; y++) {
				for (int z = mins.z; z < maxs.z; z++) {
					const glm::ivec3 p(x + sideDelta.x, y + sideDelta.y, z + sideDelta.z);
					auto iter = posMap.find(p);
					if (iter == posMap.end()) {
						posMap.emplace(p, {area, rgba});
					} else if (iter->value.entries.size() < 4 && iter->value.entries[0].color != rgba) {
						PosSampling &pos = iter->value;
						pos.entries.emplace_back(area, rgba);
					}
				}
			}
		}
	}
}

bool MeshFormat::isVoxelMesh(const TriCollection &tris) {
	for (const Tri &tri : tris) {
		if (!glm::epsilonEqual(glm::mod(tri.area(), 0.5f), 0.0f, 0.0001f)) {
			return false;
		}
		if (!tri.flat()) {
			Log::debug("No axis aligned mesh found");
#ifdef DEBUG
			for (int i = 0; i < 3; ++i) {
				const glm::vec3 &v = tri.vertices[i];
				Log::debug("tri.vertices[%i]: %f:%f:%f", i, v.x, v.y, v.z);
			}
			const glm::vec3 &n = tri.normal();
			Log::debug("tri.normal: %f:%f:%f", n.x, n.y, n.z);
#endif
			return false;
		}
	}
	Log::debug("Found axis aligned mesh");
	return true;
}

int MeshFormat::voxelizeNode(const core::String &name, scenegraph::SceneGraph &sceneGraph, const TriCollection &tris,
							 int parent, bool resetOrigin) const {
	if (tris.empty()) {
		Log::warn("Empty volume - no triangles given");
		return InvalidNodeId;
	}

	const bool axisAligned = isVoxelMesh(tris);

	glm::vec3 trisMins;
	glm::vec3 trisMaxs;
	core_assert_always(calculateAABB(tris, trisMins, trisMaxs));
	Log::debug("mins: %f:%f:%f, maxs: %f:%f:%f", trisMins.x, trisMins.y, trisMins.z, trisMaxs.x, trisMaxs.y,
			   trisMaxs.z);

	const voxel::Region region(glm::floor(trisMins), glm::ceil(trisMaxs));
	if (!region.isValid()) {
		Log::error("Invalid region: %s", region.toString().c_str());
		return InvalidNodeId;
	}

	const glm::ivec3 &vdim = region.getDimensionsInVoxels();
	if (glm::any(glm::greaterThan(vdim, glm::ivec3(512)))) {
		Log::warn("Large meshes will take a lot of time and use a lot of memory. Consider scaling the mesh! (%i:%i:%i)",
				  vdim.x, vdim.y, vdim.z);
	}
	scenegraph::SceneGraphNode node;
	node.setVolume(new voxel::RawVolume(region), true);
	node.setName(name);

	const bool fillHollow = core::Var::getSafe(cfg::VoxformatFillHollow)->boolVal();
	if (axisAligned) {
		const int maxVoxels = vdim.x * vdim.y * vdim.z;
		Log::debug("max voxels: %i (%i:%i:%i)", maxVoxels, vdim.x, vdim.y, vdim.z);
		PosMap posMap(maxVoxels);
		transformTrisAxisAligned(tris, posMap);
		voxelizeTris(node, posMap, fillHollow);
	} else {
		Log::debug("Subdivide triangles");
		core::DynamicArray<std::future<TriCollection>> futures;
		futures.reserve(tris.size());
		for (const Tri &tri : tris) {
			futures.emplace_back(app::App::getInstance()->threadPool().enqueue([tri]() {
				TriCollection subdivided;
				subdivideTri(tri, subdivided);
				return subdivided;
			}));
		}
		TriCollection subdivided;
		for (std::future<TriCollection> &future : futures) {
			const TriCollection &sub = future.get();
			subdivided.insert(subdivided.end(), sub.begin(), sub.end());
		}

		if (subdivided.empty()) {
			Log::warn("Empty volume - could not subdivide");
			return InvalidNodeId;
		}

		PosMap posMap((int)subdivided.size() * 3);
		transformTris(subdivided, posMap);
		voxelizeTris(node, posMap, fillHollow);
	}

	scenegraph::SceneGraphTransform transform;
	transform.setLocalTranslation(region.getLowerCornerf());
	scenegraph::KeyFrameIndex keyFrameIdx = 0;
	node.setTransform(keyFrameIdx, transform);

	if (resetOrigin) {
		node.volume()->translate(-region.getLowerCorner());
	}

	return sceneGraph.emplace(core::move(node), parent);
}

bool MeshFormat::calculateAABB(const TriCollection &tris, glm::vec3 &mins, glm::vec3 &maxs) {
	if (tris.empty()) {
		mins = maxs = glm::vec3(0.0f);
		return false;
	}

	maxs = tris[0].mins();
	mins = tris[0].maxs();

	for (const Tri &tri : tris) {
		maxs = glm::max(maxs, tri.maxs());
		mins = glm::min(mins, tri.mins());
	}
	return true;
}

void MeshFormat::voxelizeTris(scenegraph::SceneGraphNode &node, const PosMap &posMap, bool fillHollow) const {
	voxel::RawVolumeWrapper wrapper(node.volume());
	voxel::Palette palette;
	const bool createPalette = core::Var::getSafe(cfg::VoxelCreatePalette)->boolVal();
	if (createPalette) {
		RGBAMap colors;
		Log::debug("create palette");
		for (const auto &entry : posMap) {
			if (stopExecution()) {
				return;
			}
			const PosSampling &pos = entry->second;
			const core::RGBA rgba = pos.avgColor(_flattenFactor);
			colors.put(rgba, true);
		}
		const size_t colorCount = colors.size();
		core::Buffer<core::RGBA> colorBuffer;
		colorBuffer.reserve(colorCount);
		for (const auto &e : colors) {
			colorBuffer.push_back(e->first);
		}
		palette.quantize(colorBuffer.data(), colorBuffer.size());
	} else {
		palette = voxel::getPalette();
	}

	Log::debug("create voxels for %i positions", (int)posMap.size());
	for (const auto &entry : posMap) {
		if (stopExecution()) {
			return;
		}
		const PosSampling &pos = entry->second;
		const core::RGBA rgba = pos.avgColor(_flattenFactor);
		const voxel::Voxel voxel = voxel::createVoxel(palette, palette.getClosestMatch(rgba));
		wrapper.setVoxel(entry->first, voxel);
	}
	if (palette.colorCount() == 1) {
		if (palette.colors()[0].a == 0) {
			palette.color(0).a = 255;
		}
	}
	node.setPalette(palette);
	if (fillHollow) {
		if (stopExecution()) {
			return;
		}
		Log::debug("fill hollows");
		const voxel::Voxel voxel = voxel::createVoxel(palette, FillColorIndex);
		voxelutil::fillHollow(wrapper, voxel);
	}
}

MeshFormat::MeshExt::MeshExt(voxel::ChunkMesh *_mesh, const scenegraph::SceneGraphNode &node, bool _applyTransform)
	: mesh(_mesh), name(node.name()), applyTransform(_applyTransform) {
	size = node.region().getDimensionsInVoxels();
	nodeId = node.id();
	pivot = node.pivot();
}

bool MeshFormat::loadGroups(const core::String &filename, io::SeekableReadStream &file,
							scenegraph::SceneGraph &sceneGraph, const LoadContext &ctx) {
	const bool retVal = voxelizeGroups(filename, file, sceneGraph, ctx);
	sceneGraph.updateTransforms();
	return retVal;
}

bool MeshFormat::voxelizeGroups(const core::String &filename, io::SeekableReadStream &, scenegraph::SceneGraph &,
								const LoadContext &) {
	Log::debug("Mesh %s can't get voxelized yet", filename.c_str());
	return false;
}

core::String MeshFormat::lookupTexture(const core::String &meshFilename, const core::String &in) {
	const core::String &meshPath = core::string::extractPath(meshFilename);
	core::String name = in;
	io::normalizePath(name);
	if (!core::string::isAbsolutePath(name)) {
		name = core::string::path(meshPath, name);
	}
	if (io::filesystem()->exists(name)) {
		Log::debug("Found image %s in path %s", in.c_str(), name.c_str());
		return name;
	}

	if (!meshPath.empty()) {
		io::filesystem()->pushDir(meshPath);
	}
	core::String filename = core::string::extractFilenameWithExtension(name);
	const core::String &path = core::string::extractPath(name);
	core::String fullpath = io::searchPathFor(io::filesystem(), path, filename);
	if (fullpath.empty() && path != meshPath) {
		fullpath = io::searchPathFor(io::filesystem(), meshPath, filename);
	}
	if (fullpath.empty()) {
		fullpath = io::searchPathFor(io::filesystem(), "texture", filename);
	}
	if (fullpath.empty()) {
		fullpath = io::searchPathFor(io::filesystem(), "textures", filename);
	}

	// if not found, loop over all supported image formats and repeat the search
	if (fullpath.empty()) {
		const core::String &baseFilename = core::string::extractFilename(name);
		if (!baseFilename.empty()) {
			for (const io::FormatDescription *desc = io::format::images(); desc->valid(); ++desc) {
				for (const core::String &ext : desc->exts) {
					const core::String &f = core::string::format("%s.%s", baseFilename.c_str(), ext.c_str());
					if (f == filename) {
						continue;
					}
					fullpath = io::searchPathFor(io::filesystem(), path, f);
					if (fullpath.empty() && path != meshPath) {
						fullpath = io::searchPathFor(io::filesystem(), meshPath, f);
					}
					if (fullpath.empty()) {
						fullpath = io::searchPathFor(io::filesystem(), "texture", f);
					}
					if (fullpath.empty()) {
						fullpath = io::searchPathFor(io::filesystem(), "textures", f);
					}
					if (!fullpath.empty()) {
						if (!meshPath.empty()) {
							io::filesystem()->popDir();
						}
						return fullpath;
					}
				}
			}
		}
	}

	if (fullpath.empty()) {
		Log::error("Failed to perform texture lookup for '%s' (filename: '%s')", name.c_str(), filename.c_str());
	}
	if (!meshPath.empty()) {
		io::filesystem()->popDir();
	}
	return fullpath;
}

bool MeshFormat::saveGroups(const scenegraph::SceneGraph &sceneGraph, const core::String &filename,
							io::SeekableWriteStream &stream, const SaveContext &ctx) {
	const bool mergeQuads = core::Var::getSafe(cfg::VoxformatMergequads)->boolVal();
	const bool reuseVertices = core::Var::getSafe(cfg::VoxformatReusevertices)->boolVal();
	const bool ambientOcclusion = core::Var::getSafe(cfg::VoxformatAmbientocclusion)->boolVal();
	const bool quads = core::Var::getSafe(cfg::VoxformatQuads)->boolVal();
	const bool withColor = core::Var::getSafe(cfg::VoxformatWithColor)->boolVal();
	const bool withTexCoords = core::Var::getSafe(cfg::VoxformatWithtexcoords)->boolVal();
	const bool applyTransform = core::Var::getSafe(cfg::VoxformatTransform)->boolVal();
	const int meshMode = core::Var::getSafe(cfg::VoxelMeshMode)->intVal();
	const bool marchingCubes = meshMode == 1;

	const glm::vec3 &scale = getScale();
	const size_t models = sceneGraph.size(scenegraph::SceneGraphNodeType::AllModels);
	core::ThreadPool &threadPool = app::App::getInstance()->threadPool();
	Meshes meshes;
	core::Map<int, int> meshIdxNodeMap;
	core_trace_mutex(core::Lock, lock, "MeshFormat");
	// TODO: this could get optimized by re-using the same mesh for multiple nodes (in case of reference nodes)
	for (auto iter = sceneGraph.beginAllModels(); iter != sceneGraph.end(); ++iter) {
		const scenegraph::SceneGraphNode &node = *iter;
		auto lambda = [&, volume = sceneGraph.resolveVolume(node), region = sceneGraph.resolveRegion(node)]() {
			voxel::ChunkMesh *mesh = new voxel::ChunkMesh();
			voxel::SurfaceExtractionContext ctx =
				marchingCubes ? voxel::buildMarchingCubesContext(volume, region, *mesh, node.palette())
							  : voxel::buildCubicContext(volume, region, *mesh, glm::ivec3(0), mergeQuads,
														 reuseVertices, ambientOcclusion);
			voxel::extractSurface(ctx);
			core::ScopedLock scoped(lock);
			meshes.emplace_back(mesh, node, applyTransform);
		};
		threadPool.enqueue(lambda);
	}
	for (;;) {
		lock.lock();
		const size_t size = meshes.size();
		lock.unlock();
		if (size < models) {
			SDL_Delay(10);
		} else {
			break;
		}
	}
	Meshes nonEmptyMeshes;
	nonEmptyMeshes.reserve(meshes.size());

	// filter out empty meshes
	for (auto iter = meshes.begin(); iter != meshes.end(); ++iter) {
		if (iter->mesh->isEmpty()) {
			continue;
		}
		nonEmptyMeshes.emplace_back(*iter);
		meshIdxNodeMap.put(iter->nodeId, (int)nonEmptyMeshes.size() - 1);
	}
	bool state;
	if (nonEmptyMeshes.empty()) {
		Log::warn("Empty scene can't get saved as mesh");
		state = false;
	} else {
		Log::debug("Save meshes");
		state = saveMeshes(meshIdxNodeMap, sceneGraph, nonEmptyMeshes, filename, stream, scale,
						   marchingCubes ? false : quads, withColor, withTexCoords);
	}
	for (MeshExt &meshext : meshes) {
		delete meshext.mesh;
	}
	return state;
}

} // namespace voxelformat
