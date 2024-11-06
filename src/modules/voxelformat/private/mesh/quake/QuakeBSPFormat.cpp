/**
 * @file
 */

#include "QuakeBSPFormat.h"
#include "core/Common.h"
#include "core/FourCC.h"
#include "core/Log.h"
#include "core/RGBA.h"
#include "core/ScopedPtr.h"
#include "core/StandardLib.h"
#include "core/StringUtil.h"
#include "core/collection/Buffer.h"
#include "core/collection/DynamicArray.h"
#include "image/Image.h"
#include "scenegraph/SceneGraph.h"
#include "palette/PaletteLookup.h"
#include "voxelutil/VoxelUtil.h"

namespace voxelformat {

namespace _priv {

static const uint32_t ufoaiEntitiesLump = 0;
static const uint32_t ufoaiVerticesLump = 2;
static const uint32_t ufoaiTexinfoLump = 5;
static const uint32_t ufoaiFacesLump = 6;
static const uint32_t ufoaiEdgesLump = 11;
static const uint32_t ufoaiSurfedgesLump = 12;
static const uint32_t ufoaiModelsLump = 13;

static const uint32_t quake1VerticesLump = 3;
static const uint32_t quake1TexturesLump = 2;
static const uint32_t quake1TexinfoLump = 6;
static const uint32_t quake1FacesLump = 7;
static const uint32_t quake1EdgesLump = 12;
static const uint32_t quake1SurfedgesLump = 13;

} // namespace _priv

#define wrap(read)                                                                                                     \
	if ((read) != 0) {                                                                                                 \
		Log::error("Failed to read bsp " CORE_STRINGIFY(read));                                                        \
		return false;                                                                                                  \
	}

int32_t QuakeBSPFormat::validateLump(const BspLump &lump, size_t elementSize) const {
	if (lump.len % elementSize) {
		Log::error("Unexpected lump size found: %u versus element size %u", lump.len, (uint32_t)elementSize);
		return -1;
	}
	const uint32_t count = lump.len / elementSize;
	Log::debug("%u entries found in lump at offset %u of size %u", count, lump.offset, lump.len);
	return (int32_t)count;
}

static core::String extractBaseDir(const core::String &filename) {
	const size_t pos = filename.rfind("maps/");
	if (pos == core::String::npos) {
		return "";
	}
	return filename.substr(0, pos);
}

bool QuakeBSPFormat::loadQuake1Textures(const core::String &filename, io::SeekableReadStream &stream,
										const BspHeader &header, core::DynamicArray<Texture> &textures,
										core::StringMap<image::ImagePtr> &textureMap) {
	core::DynamicArray<Quake1Texinfo> miptex;

	struct TextureLump {
		int32_t nummiptex;
		core::DynamicArray<int32_t> dataofs;
	};

	TextureLump m;
	const uint32_t baseOffset = header.lumps[_priv::quake1TexturesLump].offset;
	{
		if (stream.seek(baseOffset) == -1) {
			Log::error("Invalid texture lump offset - can't seek");
			return false;
		}

		stream.readInt32(m.nummiptex);
		m.dataofs.resize(m.nummiptex);
		for (int i = 0; i < m.nummiptex; ++i) {
			stream.readInt32(m.dataofs[i]);
		}

		miptex.resize(m.nummiptex);
		for (int i = 0; i < m.nummiptex; ++i) {
			if (m.dataofs[i] == -1) {
				continue;
			}
			if (stream.seek(baseOffset + m.dataofs[i]) == -1) {
				Log::error("Invalid texinfo offset - can't seek (%i)", m.dataofs[i]);
				return false;
			}

			Quake1Texinfo &mt = miptex[i];
			stream.readString(sizeof(mt.name), mt.name, false);
			stream.readUInt32(mt.width);
			stream.readUInt32(mt.height);
			stream.readUInt32(mt.offset1);
			stream.readUInt32(mt.offset2);
			stream.readUInt32(mt.offset4);
			stream.readUInt32(mt.offset8);
		}
	}

	const int32_t texInfoCount = validateLump(header.lumps[_priv::quake1TexinfoLump], sizeof(BspTextureBase));
	if (texInfoCount <= 0) {
		Log::error("Invalid bsp file with no textures in lump");
		return false;
	}

	if (stream.seek(header.lumps[_priv::quake1TexinfoLump].offset) == -1) {
		Log::error("Invalid texture lump offset - can't seek");
		return false;
	}

	textures.resize(texInfoCount);
	for (int32_t i = 0; i < texInfoCount; i++) {
		Texture &texture = textures[i];
		for (int j = 0; j < 3; ++j) {
			wrap(stream.readFloat(texture.vecS[j]))
		}
		wrap(stream.readFloat(texture.distS))
		for (int j = 0; j < 3; ++j) {
			wrap(stream.readFloat(texture.vecT[j]))
		}
		wrap(stream.readFloat(texture.distT))
		wrap(stream.readUInt32(texture.surfaceFlags)) // miptex index
		wrap(stream.readUInt32(texture.value)) // 0 = solid, 1 = water
		SDL_strlcpy(texture.name, miptex[texture.surfaceFlags].name, sizeof(texture.name));
	}

	palette::Palette pal;
	pal.quake1();

	for (int32_t i = 0; i < texInfoCount; i++) {
		Texture &texture = textures[i];

		auto iter = textureMap.find(texture.name);
		if (iter != textureMap.end()) {
			texture.image = iter->second;
			continue;
		}

		Quake1Texinfo &texinfo = miptex[texture.surfaceFlags];

		image::ImagePtr tex = image::createEmptyImage(texture.name);
		const int width = (int)texinfo.width;
		const int height = (int)texinfo.height;

		if (stream.seek(baseOffset + m.dataofs[texture.surfaceFlags] + (int)texinfo.offset1) == -1) {
			Log::error("Failed to seek to pixel data %i", i);
			continue;
		}
		const int pixelSize = width * height;
		uint8_t *pixels = (uint8_t *)core_malloc(pixelSize);
		if (stream.read(pixels, pixelSize) == -1) {
			Log::error("Failed to read %i bytes to pixel data %i", pixelSize, i);
			continue;
		}

		core::Buffer<core::RGBA> buffer(pixelSize);
		for (int j = 0; j < pixelSize; ++j) {
			buffer[j] = pal.color(pixels[j]);
		}
		if (tex->loadRGBA((const uint8_t *)buffer.data(), width, height)) {
			Log::debug("Use image %s", texture.name);
			textureMap.put(texture.name, tex);
			texture.image = tex;
		} else {
			Log::warn("Failed to load %s", texture.name);
		}
	}
	return true;
}

bool QuakeBSPFormat::loadUFOAlienInvasionTextures(const core::String &filename, io::SeekableReadStream &stream,
												  const BspHeader &header, core::DynamicArray<Texture> &textures,
												  core::StringMap<image::ImagePtr> &textureMap) {
	const int32_t textureCount = validateLump(header.lumps[_priv::ufoaiTexinfoLump], sizeof(BspTexture));
	if (textureCount <= 0) {
		Log::error("Invalid bsp file with no textures in lump");
		return false;
	}

	if (stream.seek(header.lumps[_priv::ufoaiTexinfoLump].offset) == -1) {
		Log::error("Invalid texture lump offset - can't seek");
		return false;
	}

	textures.resize((size_t)textureCount);
	for (int32_t i = 0; i < textureCount; i++) {
		Texture &texture = textures[i];
		for (int j = 0; j < 3; ++j) {
			wrap(stream.readFloat(texture.vecS[j]))
		}
		wrap(stream.readFloat(texture.distS))
		for (int j = 0; j < 3; ++j) {
			wrap(stream.readFloat(texture.vecT[j]))
		}
		wrap(stream.readFloat(texture.distT))
		wrap(stream.readUInt32(texture.surfaceFlags))
		wrap(stream.readUInt32(texture.value))
		if (!stream.readString(lengthof(texture.name), texture.name, false)) {
			Log::error("Failed to read bsp - texture name invalid");
			return false;
		}

		auto iter = textureMap.find(texture.name);
		if (iter != textureMap.end()) {
			Log::debug("texture for material '%s' is already loaded", texture.name);
			texture.image = iter->second;
			continue;
		}

		core::String textureName = texture.name;
		const core::String &path = extractBaseDir(filename);
		textureName = core::string::path(path, "textures", textureName);
		Log::debug("Search image %s in path %s", textureName.c_str(), path.c_str());
		image::ImagePtr tex = image::loadImage(textureName);
		if (tex->isLoaded()) {
			Log::debug("Use image %s", textureName.c_str());
			textureMap.put(textureName, tex);
			texture.image = tex;
		} else {
			Log::warn("Failed to load %s", textureName.c_str());
		}
	}
	return true;
}

bool QuakeBSPFormat::loadQuake1Faces(io::SeekableReadStream &stream, const BspHeader &header,
									 core::DynamicArray<Face> &faces, const core::DynamicArray<Texture> &textures) {
	const int32_t faceCount = validateLump(header.lumps[_priv::quake1FacesLump], sizeof(BspFace));
	if (faceCount <= 0) {
		Log::error("Invalid bsp file with no faces in lump");
		return false;
	}
	if (stream.seek(header.lumps[_priv::quake1FacesLump].offset) == -1) {
		Log::error("Invalid faces lump offset - can't seek");
		return false;
	}
	faces.reserve(faceCount);
	for (int32_t i = 0; i < faceCount; i++) {
		stream.skip(2); // planeId
		stream.skip(2); // side

		Face face;
		wrap(stream.readInt32(face.edgeId))
		wrap(stream.readInt16(face.edgeCount))
		wrap(stream.readInt16(face.textureId))

		const char *texName = textures[face.textureId].name;
		if (!core::string::startsWith(texName, "sky") /* && texName[0] != '*'*/) {
			faces.push_back(face);
		} else {
			Log::debug("skip face with %s", texName);
		}

		uint8_t lightType; // type of lighting, for the face
		uint8_t baselight; // from 0xFF (dark) to 0 (bright)
		uint8_t light[2];  // two additional light models
		int32_t lightmap;  // Pointer inside the general light map, or -1
						   // this define the start of the face light map

		wrap(stream.readUInt8(lightType))
		wrap(stream.readUInt8(baselight))
		wrap(stream.readUInt8(light[0]))
		wrap(stream.readUInt8(light[1]))

		wrap(stream.readInt32(lightmap))
	}
	Log::debug("Loaded %i faces", faceCount);
	return !faces.empty();
}

bool QuakeBSPFormat::loadUFOAlienInvasionFaces(io::SeekableReadStream &stream, const BspHeader &header,
											   core::DynamicArray<Face> &faces) {
	const int32_t faceCount = validateLump(header.lumps[_priv::ufoaiFacesLump], sizeof(BspFace));
	if (faceCount <= 0) {
		Log::error("Invalid bsp file with no faces in lump");
		return false;
	}
	if (stream.seek(header.lumps[_priv::ufoaiFacesLump].offset) == -1) {
		Log::error("Invalid faces lump offset - can't seek");
		return false;
	}
	faces.resize(faceCount);
	for (int32_t i = 0; i < faceCount; i++) {
		stream.skip(2); // planeId
		stream.skip(2); // side

		Face &face = faces[i];
		wrap(stream.readInt32(face.edgeId))
		wrap(stream.readInt16(face.edgeCount))
		wrap(stream.readInt16(face.textureId))

		stream.skip(4); // lightofsDay
		stream.skip(4); // lightofsNight
	}
	Log::debug("Loaded %i faces", faceCount);
	return true;
}

bool QuakeBSPFormat::loadUFOAlienInvasionFacesForLevel(io::SeekableReadStream &stream, const BspHeader &header,
													   core::DynamicArray<Face> &faces,
													   core::DynamicArray<Face> &facesLevel,
													   const core::DynamicArray<Model> &models, int level) {
	const uint32_t size = models.size();
	if (size < 255) {
		return false;
	}
	const uint32_t mask = 1u << level;
	// a face that is in level 1, 2 and 3 is in model 7 - visible everywhere is 255
	// not marked for any level is 0 (we are skipping this)
	for (uint32_t i = 0; i <= 255; ++i) {
		if (i && !(i & mask)) {
			continue;
		}
		const int32_t begin = models[i].faceId;
		const int32_t end = begin + models[i].faceCount;
		for (int32_t f = begin; f < end; ++f) {
			core_assert_msg(f >= 0 && f < (int)faces.size(), "Face index is out of bounds: %i vs %i", f,
							(int)faces.size());
			Face &face = faces[f];
			if (face.used) {
				continue;
			}
			facesLevel.push_back(face);
			face.used = true;
		}
	}
	return true;
}

bool QuakeBSPFormat::loadUFOAlienInvasionEdges(io::SeekableReadStream &stream, const BspHeader &header,
											   core::DynamicArray<BspEdge> &edges,
											   core::DynamicArray<int32_t> &surfEdges) {
	const int32_t edgeCount = validateLump(header.lumps[_priv::ufoaiEdgesLump], sizeof(BspEdge));
	if (edgeCount <= 0) {
		Log::error("Invalid bsp file with no edges in lump");
		return false;
	}
	if (stream.seek(header.lumps[_priv::ufoaiEdgesLump].offset) == -1) {
		Log::error("Invalid edges lump offset - can't seek");
		return false;
	}
	edges.resize(edgeCount);
	for (int32_t i = 0; i < edgeCount; i++) {
		wrap(stream.readInt16(edges[i].vertexIndices[0]))
		wrap(stream.readInt16(edges[i].vertexIndices[1]))
	}
	Log::debug("Loaded %i edges", edgeCount);

	const int32_t surfEdgesCount = validateLump(header.lumps[_priv::ufoaiSurfedgesLump], sizeof(BspEdge));
	if (surfEdgesCount <= 0) {
		Log::error("Invalid bsp file with no surfedges in lump");
		return false;
	}
	if (stream.seek(header.lumps[_priv::ufoaiSurfedgesLump].offset) == -1) {
		Log::error("Invalid surfedges lump offset - can't seek");
		return false;
	}
	surfEdges.resize(surfEdgesCount);
	for (int32_t i = 0; i < surfEdgesCount; i++) {
		wrap(stream.readInt32(surfEdges[i]))
	}
	Log::debug("Loaded %i surfedges", surfEdgesCount);

	return true;
}

bool QuakeBSPFormat::loadQuake1Edges(io::SeekableReadStream &stream, const BspHeader &header,
									 core::DynamicArray<BspEdge> &edges, core::DynamicArray<int32_t> &surfEdges) {
	const int32_t edgeCount = validateLump(header.lumps[_priv::quake1EdgesLump], sizeof(BspEdge));
	if (edgeCount <= 0) {
		Log::error("Invalid bsp file with no edges in lump");
		return false;
	}
	if (stream.seek(header.lumps[_priv::quake1EdgesLump].offset) == -1) {
		Log::error("Invalid edges lump offset - can't seek");
		return false;
	}
	edges.resize(edgeCount);
	for (int32_t i = 0; i < edgeCount; i++) {
		wrap(stream.readInt16(edges[i].vertexIndices[0]))
		wrap(stream.readInt16(edges[i].vertexIndices[1]))
	}
	Log::debug("Loaded %i edges", edgeCount);

	const int32_t surfEdgesCount = validateLump(header.lumps[_priv::quake1SurfedgesLump], sizeof(BspEdge));
	if (surfEdgesCount <= 0) {
		Log::error("Invalid bsp file with no surfedges in lump");
		return false;
	}
	if (stream.seek(header.lumps[_priv::quake1SurfedgesLump].offset) == -1) {
		Log::error("Invalid surfedges lump offset - can't seek");
		return false;
	}
	surfEdges.resize(surfEdgesCount);
	for (int32_t i = 0; i < surfEdgesCount; i++) {
		wrap(stream.readInt32(surfEdges[i]))
	}
	Log::debug("Loaded %i surfedges", surfEdgesCount);

	return true;
}

bool QuakeBSPFormat::loadUFOAlienInvasionVertices(io::SeekableReadStream &stream, const BspHeader &header,
												  core::DynamicArray<BspVertex> &vertices) {
	const int32_t vertexCount = validateLump(header.lumps[_priv::ufoaiVerticesLump], sizeof(BspVertex));
	if (vertexCount <= 0) {
		return false;
	}
	if (stream.seek(header.lumps[_priv::ufoaiVerticesLump].offset) == -1) {
		Log::error("Invalid vertices lump offset - can't seek");
		return false;
	}
	vertices.resize(vertexCount);
	for (int32_t i = 0; i < vertexCount; i++) {
		wrap(stream.readFloat(vertices[i].x))
		wrap(stream.readFloat(vertices[i].y))
		wrap(stream.readFloat(vertices[i].z))
	}
	return true;
}

bool QuakeBSPFormat::loadQuake1Vertices(io::SeekableReadStream &stream, const BspHeader &header,
										core::DynamicArray<BspVertex> &vertices) {
	const int32_t vertexCount = validateLump(header.lumps[_priv::quake1VerticesLump], sizeof(BspVertex));
	if (vertexCount <= 0) {
		return false;
	}
	if (stream.seek(header.lumps[_priv::quake1VerticesLump].offset) == -1) {
		Log::error("Invalid vertices lump offset - can't seek");
		return false;
	}
	vertices.resize(vertexCount);
	for (int32_t i = 0; i < vertexCount; i++) {
		wrap(stream.readFloat(vertices[i].x))
		wrap(stream.readFloat(vertices[i].y))
		wrap(stream.readFloat(vertices[i].z))
	}
	return true;
}

bool QuakeBSPFormat::loadQuake1Bsp(const core::String &filename, io::SeekableReadStream &stream,
								   scenegraph::SceneGraph &sceneGraph, const BspHeader &header) {
	core::StringMap<image::ImagePtr> textureMap;
	core::DynamicArray<Texture> textures;
	if (!loadQuake1Textures(filename, stream, header, textures, textureMap)) {
		Log::error("Failed to load textures");
		return false;
	}

	core::DynamicArray<Face> faces;
	if (!loadQuake1Faces(stream, header, faces, textures)) {
		Log::error("Failed to load faces");
		return false;
	}

	core::DynamicArray<BspEdge> edges;
	core::DynamicArray<int32_t> surfEdges;
	if (!loadQuake1Edges(stream, header, edges, surfEdges)) {
		Log::error("Failed to load edges");
		return false;
	}

	core::DynamicArray<BspVertex> vertices;
	if (!loadQuake1Vertices(stream, header, vertices)) {
		Log::error("Failed to load vertices");
		return false;
	}

	palette::PaletteLookup palLookup;
	palLookup.palette().quake1();
	const core::String &name = core::string::extractFilename(filename);
	if (!voxelize(textures, faces, edges, surfEdges, vertices, sceneGraph, palLookup, name)) {
		Log::error("Failed to voxelize %s", filename.c_str());
		return false;
	}
	sceneGraph.updateTransforms();
	return true;
}

bool QuakeBSPFormat::loadUFOAlienInvasionModels(io::SeekableReadStream &stream, const BspHeader &header,
												core::DynamicArray<Model> &models) {
	const int32_t modelCount = validateLump(header.lumps[_priv::ufoaiModelsLump], sizeof(BspModel));
	if (modelCount <= 0) {
		Log::error("Invalid bsp file with no models in lump");
		return false;
	}
	if (stream.seek(header.lumps[_priv::ufoaiModelsLump].offset) == -1) {
		Log::error("Invalid models lump offset - can't seek");
		return false;
	}
	models.resize(modelCount);

	const int64_t modelSkipSize = 9 * sizeof(float) + sizeof(int32_t);
	static_assert(sizeof(BspModel) == modelSkipSize + 2 * sizeof(int32_t), "Unexpected BspModel structure size");
	for (int32_t i = 0; i < modelCount; ++i) {
		if (stream.skip(modelSkipSize) == -1) {
			Log::error("Failed to read model %i", i);
			return false;
		}
		Model &mdl = models[i];
		wrap(stream.readInt32(mdl.faceId))
		wrap(stream.readInt32(mdl.faceCount))
		if (mdl.faceCount == 0) {
			Log::debug("model %i (of %i) has no faces", i, modelCount);
		}
	}
	Log::debug("Loaded %i models", modelCount);
	return true;
}

static int parseMaxLevel(const core::String &entities) {
	int maxLevel = 8;
	const size_t start = entities.find("\"maxlevel\"");
	if (start == core::String::npos) {
		Log::debug("No maxlevel found in worldspawn");
		return maxLevel;
	}
	const size_t end = entities.find("\n", start);
	if (end == core::String::npos) {
		Log::warn("Invalid maxlevel found in worldspawn");
		return maxLevel;
	}
	core::String line = entities.substr(start, end).trim();
	line = line.substr(12);
	maxLevel = core::string::toInt(line);
	if (maxLevel == 0) {
		maxLevel = 8;
	}
	Log::debug("Maxlevel: %i", maxLevel);
	return maxLevel;
}

bool QuakeBSPFormat::loadUFOAlienInvasionBsp(const core::String &filename, io::SeekableReadStream &stream,
											 scenegraph::SceneGraph &sceneGraph, const BspHeader &header) {
	Log::debug("Load textures");
	core::StringMap<image::ImagePtr> textureMap;
	core::DynamicArray<Texture> textures;
	if (!loadUFOAlienInvasionTextures(filename, stream, header, textures, textureMap)) {
		Log::error("Failed to load textures");
		return false;
	}

	Log::debug("Load faces");
	core::DynamicArray<Face> faces;
	if (!loadUFOAlienInvasionFaces(stream, header, faces)) {
		Log::error("Failed to load faces");
		return false;
	}

	Log::debug("Load edges");
	core::DynamicArray<BspEdge> edges;
	core::DynamicArray<int32_t> surfEdges;
	if (!loadUFOAlienInvasionEdges(stream, header, edges, surfEdges)) {
		Log::error("Failed to load edges");
		return false;
	}

	Log::debug("Load vertices");
	core::DynamicArray<BspVertex> vertices;
	if (!loadUFOAlienInvasionVertices(stream, header, vertices)) {
		Log::error("Failed to load vertices");
		return false;
	}

	Log::debug("Load models");
	core::DynamicArray<Model> models;
	if (!loadUFOAlienInvasionModels(stream, header, models)) {
		Log::error("Failed to load models");
		return false;
	}

	Log::debug("Load entities");
	stream.seek(header.lumps[_priv::ufoaiEntitiesLump].offset);
	core::String entities(header.lumps[_priv::ufoaiEntitiesLump].len, ' ');
	stream.readString((int)header.lumps[_priv::ufoaiEntitiesLump].len, entities.c_str());
	const int maxLevel = parseMaxLevel(entities);

	bool state = false;

	// make one palette for all 8 levels
	palette::PaletteLookup palLookup;

	core::DynamicArray<Face> facesLevel;
	for (int i = 0; i < maxLevel; ++i) {
		Log::debug("Load level %i/%i", i, maxLevel);
		facesLevel.clear();
		if (!loadUFOAlienInvasionFacesForLevel(stream, header, faces, facesLevel, models, i)) {
			Log::debug("No content at level %i - skipping", i);
			continue;
		}
		Log::debug("Voxelize level %i", i);
		if (voxelize(textures, facesLevel, edges, surfEdges, vertices, sceneGraph, palLookup,
					 core::string::format("Level %i", i + 1))) {
			state = true;
		}
	}
	sceneGraph.updateTransforms();
	return state;
}

bool QuakeBSPFormat::voxelize(const core::DynamicArray<Texture> &textures, const core::DynamicArray<Face> &faces,
							  const core::DynamicArray<BspEdge> &edges, const core::DynamicArray<int32_t> &surfEdges,
							  const core::DynamicArray<BspVertex> &vertices, scenegraph::SceneGraph &sceneGraph,
							  palette::PaletteLookup &palLookup, const core::String &name) {
	int vertexCount = 0;
	int indexCount = 0;
	for (const Face &face : faces) {
		vertexCount += face.edgeCount;
		if (face.edgeCount > 2) {
			indexCount += (face.edgeCount - 2) * 3;
		}
	}

	Log::debug("Prepare voxeliziation bsp with %i vertices", vertexCount);

	core::DynamicArray<glm::vec2> texcoords(vertexCount);
	core::DynamicArray<glm::vec3> verts(vertexCount);
	core::DynamicArray<int32_t> textureIndices(vertexCount);
	core::DynamicArray<int32_t> indices(indexCount);

	int offset = 0;
	for (Face &face : faces) {
		face.index = offset;

		for (int j = 0; j < face.edgeCount; j++) {
			if (stopExecution()) {
				break;
			}
			const int surfEdgesIdx = face.edgeId + j;
			if (surfEdgesIdx < 0 || surfEdgesIdx >= (int)surfEdges.size()) {
				Log::error("Invalid surf edge index given: %i", surfEdgesIdx);
				return false;
			}
			const int index = surfEdges[surfEdgesIdx];

			const BspVertex *vert;
			if (index > 0) {
				const BspEdge *edge = &edges[index];
				vert = &vertices[edge->vertexIndices[0]];
			} else {
				const BspEdge *edge = &edges[-index];
				vert = &vertices[edge->vertexIndices[1]];
			}

			const Texture &texture = textures[face.textureId];
			const glm::vec3 sdir(texture.vecS[0], texture.vecS[1], texture.vecS[2]);
			const glm::vec3 tdir(texture.vecT[0], texture.vecT[1], texture.vecT[2]);

			const glm::vec3 vertex(vert->x, vert->y, vert->z);
			if (texture.image) {
				/* texture coordinates */
				float s = glm::dot(vertex, sdir) + texture.distS;
				s /= (float)texture.image->width();

				float t = glm::dot(vertex, tdir) + texture.distT;
				t /= (float)texture.image->height();

				texcoords[offset] = glm::vec2(s, t);
			} else {
				texcoords[offset] = glm::vec2(0, 0);
			}
			textureIndices[offset] = face.textureId;
			verts[offset] = vertex;
			core::exchange(verts[offset].y, verts[offset].z);
			++offset;
		}
	}
	int numIndices = 0;
	for (const Face &face : faces) {
		if (face.edgeCount <= 2) {
			continue;
		}
		// triangle fan
		const int numTris = face.edgeCount - 2;
		for (int k = 0; k < numTris; k++) {
			indices[numIndices++] = face.index;
			indices[numIndices++] = face.index + k + 1;
			indices[numIndices++] = face.index + k + 2;
		}
	}

	core_assert(numIndices == indexCount);

	Log::debug("Voxelize bsp with %i vertices", vertexCount);

	const glm::vec3 &scale = getInputScale();
	TriCollection tris;
	tris.reserve(numIndices / 3);
	for (int i = 0; i < numIndices; i += 3) {
		if (stopExecution()) {
			break;
		}
		voxelformat::TexturedTri tri;
		for (int k = 0; k < 3; ++k) {
			const int idx = indices[i + k];
			tri.vertices[k] = verts[idx] * scale;
			tri.uv[k] = texcoords[idx];
		}
		const int textureIdx = textureIndices[indices[i]];
		const Texture &texture = textures[textureIdx];
		tri.texture = texture.image;
		tris.push_back(tri);
	}

	return voxelizeNode(name, sceneGraph, tris) > 0;
}

bool QuakeBSPFormat::voxelizeGroups(const core::String &filename, const io::ArchivePtr &archive,
									scenegraph::SceneGraph &sceneGraph, const LoadContext &ctx) {
	core::ScopedPtr<io::SeekableReadStream> stream(archive->readStream(filename));
	if (!stream) {
		Log::error("Could not load file %s", filename.c_str());
		return false;
	}
	static const uint32_t q1Version = FourCC('\x1d', '\0', '\0', '\0');
	static const uint32_t bspMagic = FourCC('I', 'B', 'S', 'P');

	BspHeader header;
	wrap(stream->readUInt32(header.magic))
	if (header.magic == q1Version) {
		header.version = 29;
	} else {
		wrap(stream->readUInt32(header.version))
	}
	for (int i = 0; i < lengthof(header.lumps); ++i) {
		wrap(stream->readUInt32(header.lumps[i].offset))
		wrap(stream->readUInt32(header.lumps[i].len))
	}

	if (header.version == 79 && header.magic == bspMagic) {
		return loadUFOAlienInvasionBsp(filename, *stream, sceneGraph, header);
	}
	if (header.magic == q1Version) {
		return loadQuake1Bsp(filename, *stream, sceneGraph, header);
	}

	uint8_t buf[4];
	FourCCRev(buf, header.magic);
	Log::error("Unsupported bsp file with magic %c%c%c%c and version %i", buf[0], buf[1], buf[2], buf[3],
			   header.version);
	return false;
}

#undef wrap

} // namespace voxelformat
