/**
 * @file
 */

#include "VolumeFormat.h"
#include "app/App.h"
#include "core/Log.h"
#include "core/ScopedPtr.h"
#include "core/SharedPtr.h"
#include "core/StringUtil.h"
#include "core/Trace.h"
#include "core/collection/DynamicArray.h"
#include "io/Archive.h"
#include "io/File.h"
#include "io/FilesystemArchive.h"
#include "io/FormatDescription.h"
#include "io/Stream.h"
#include "metric/MetricFacade.h"
#include "palette/PaletteFormatDescription.h"
#include "scenegraph/SceneGraphNode.h"
#include "video/Texture.h"
#include "voxelformat/Format.h"
#include "voxelformat/private/aceofspades/AoSVXLFormat.h"
#include "voxelformat/private/animatoon/AnimaToonFormat.h"
#include "voxelformat/private/benvoxel/BenVoxelFormat.h"
#include "voxelformat/private/image/AsepriteFormat.h"
#include "voxelformat/private/binvox/BinVoxFormat.h"
#include "voxelformat/private/chronovox/CSMFormat.h"
#include "voxelformat/private/commandconquer/VXLFormat.h"
#include "voxelformat/private/cubeworld/CubFormat.h"
#include "voxelformat/private/cubzh/CubzhB64Format.h"
#include "voxelformat/private/cubzh/CubzhFormat.h"
#include "voxelformat/private/cubzh/PCubesFormat.h"
#include "voxelformat/private/goxel/GoxFormat.h"
#include "voxelformat/private/image/PNGFormat.h"
#include "voxelformat/private/magicavoxel/VoxFormat.h"
#include "voxelformat/private/magicavoxel/XRawFormat.h"
#include "voxelformat/private/mesh/Autodesk3DSFormat.h"
#include "voxelformat/private/mesh/BlockbenchFormat.h"
#include "voxelformat/private/mesh/FBXFormat.h"
#include "voxelformat/private/mesh/GLTFFormat.h"
#include "voxelformat/private/mesh/OBJFormat.h"
#include "voxelformat/private/mesh/PLYFormat.h"
#include "voxelformat/private/mesh/STLFormat.h"
#include "voxelformat/private/mesh/quake/MD2Format.h"
#include "voxelformat/private/mesh/quake/MDLFormat.h"
#include "voxelformat/private/mesh/quake/MapFormat.h"
#include "voxelformat/private/mesh/quake/QuakeBSPFormat.h"
#include "voxelformat/private/minecraft/DatFormat.h"
#include "voxelformat/private/minecraft/MCRFormat.h"
#include "voxelformat/private/minecraft/MTSFormat.h"
#include "voxelformat/private/minecraft/SchematicFormat.h"
#include "voxelformat/private/qubicle/QBCLFormat.h"
#include "voxelformat/private/qubicle/QBFormat.h"
#include "voxelformat/private/qubicle/QBTFormat.h"
#include "voxelformat/private/qubicle/QEFFormat.h"
#include "voxelformat/private/rooms/ThingFormat.h"
#include "voxelformat/private/sandbox/VXBFormat.h"
#include "voxelformat/private/sandbox/VXCFormat.h"
#include "voxelformat/private/sandbox/VXMFormat.h"
#include "voxelformat/private/sandbox/VXRFormat.h"
#include "voxelformat/private/sandbox/VXTFormat.h"
#include "voxelformat/private/slab6/KV6Format.h"
#include "voxelformat/private/slab6/KVXFormat.h"
#include "voxelformat/private/slab6/SLAB6VoxFormat.h"
#include "voxelformat/private/spritestack/SpriteStackFormat.h"
#include "voxelformat/private/sproxel/SproxelFormat.h"
#include "voxelformat/private/starmade/SMFormat.h"
#include "voxelformat/private/starmade/SMTPLFormat.h"
#include "voxelformat/private/vengi/VENGIFormat.h"
#include "voxelformat/private/voxel3d/V3AFormat.h"
#include "voxelformat/private/voxelbuilder/VBXFormat.h"
#include "voxelformat/private/voxelmax/VMaxFormat.h"

namespace voxelformat {

const io::FormatDescription *voxelLoad() {
	// this is the list of supported voxel volume formats that
	// have importers implemented
	static const io::FormatDescription desc[] = {VENGIFormat::format(),
												 AsepriteFormat::format(),
												 QBFormat::format(),
												 BenVoxelFormat::format(),
												 VoxFormat::format(),
												 QBTFormat::format(),
												 QBCLFormat::format(),
												 VXTFormat::format(),
												 VXCFormat::format(),
												 VXMFormat::format(),
												 VXBFormat::format(),
												 BlockbenchFormat::format(),
												 VXRFormat::format(),
												 BinVoxFormat::format(),
												 GoxFormat::format(),
												 CubFormat::format(),
												 MTSFormat::format(),
												 MCRFormat::format(),
												 DatFormat::format(),
												 SchematicFormat::format(),
												 MapFormat::format(),
												 QuakeBSPFormat::formatUFOAI(),
												 QuakeBSPFormat::formatQuake1(),
												 MDLFormat::format(),
												 MD2Format::format(),
												 FBXFormat::format(),
												 Autodesk3DSFormat::format(),
												 SproxelFormat::format(),
												 XRawFormat::format(),
												 SMFormat::format(),
												 SMTPLFormat::format(),
												 AnimaToonFormat::format(),
												 VBXFormat::format(),
												 OBJFormat::format(),
												 GLTFFormat::format(),
												 STLFormat::format(),
												 PLYFormat::format(),
												 KVXFormat::format(),
												 PCubesFormat::format(),
												 CubzhFormat::format(),
												 CubzhB64Format::format(),
												 V3AFormat::format(),
												 ThingFormat::format(),
												 KV6Format::format(),
												 VXLFormat::format(),
												 AoSVXLFormat::format(),
												 QEFFormat::format(),
												 CSMFormat::format(),
												 CSMFormat::formatNVM(),
												 SLAB6VoxFormat::format(),
												 SpriteStackFormat::format(),
												 VMaxFormat::format(),
												 io::format::png(),
												 {"", {}, {}, 0u}};
	return desc;
}

const io::FormatDescription *voxelSave() {
	static core::DynamicArray<io::FormatDescription> desc;
	if (desc.empty()) {
		for (const io::FormatDescription *d = voxelformat::voxelLoad(); d->valid(); ++d) {
			if (d->flags & FORMAT_FLAG_SAVE) {
				desc.push_back(*d);
			}
		}
		desc.push_back({"", {}, {}, 0u});
	}
	return desc.data();
}

static core::SharedPtr<Format> getFormat(const io::FormatDescription &desc, uint32_t magic) {
	for (const core::String &ext : desc.exts) {
		// you only have to check one of the supported extensions
		// here
		if (ext == VENGIFormat::format().mainExtension()) {
			return core::make_shared<VENGIFormat>();
		} else if (ext == QBFormat::format().mainExtension()) {
			return core::make_shared<QBFormat>();
		} else if (ext == VoxFormat::format().mainExtension() && desc.name == VoxFormat::format().name) {
			return core::make_shared<VoxFormat>();
		} else if (ext == SLAB6VoxFormat::format().mainExtension()) {
			return core::make_shared<SLAB6VoxFormat>();
		} else if (ext == QBTFormat::format().mainExtension() || io::isA(QBTFormat::format(), magic)) {
			return core::make_shared<QBTFormat>();
		} else if (ext == KVXFormat::format().mainExtension()) {
			return core::make_shared<KVXFormat>();
		} else if (ext == KV6Format::format().mainExtension()) {
			return core::make_shared<KV6Format>();
		} else if (ext == SproxelFormat::format().mainExtension()) {
			return core::make_shared<SproxelFormat>();
		} else if (ext == CubFormat::format().mainExtension()) {
			return core::make_shared<CubFormat>();
		} else if (ext == GoxFormat::format().mainExtension()) {
			return core::make_shared<GoxFormat>();
		} else if (ext == AnimaToonFormat::format().mainExtension()) {
			return core::make_shared<AnimaToonFormat>();
		} else if (ext == MCRFormat::format().mainExtension()) {
			return core::make_shared<MCRFormat>();
		} else if (ext == MTSFormat::format().mainExtension()) {
			return core::make_shared<MTSFormat>();
		} else if (ext == DatFormat::format().mainExtension()) {
			return core::make_shared<DatFormat>();
		} else if (ext == SMFormat::format().mainExtension()) {
			return core::make_shared<SMFormat>();
		} else if (ext == SMTPLFormat::format().mainExtension()) {
			return core::make_shared<SMTPLFormat>();
		} else if (ext == VXMFormat::format().mainExtension()) {
			return core::make_shared<VXMFormat>();
		} else if (ext == VXRFormat::format().mainExtension()) {
			return core::make_shared<VXRFormat>();
		} else if (ext == VXBFormat::format().mainExtension()) {
			return core::make_shared<VXBFormat>();
		} else if (ext == VMaxFormat::format().mainExtension()) {
			return core::make_shared<VMaxFormat>();
		} else if (ext == BlockbenchFormat::format().mainExtension()) {
			return core::make_shared<BlockbenchFormat>();
		} else if (ext == VXCFormat::format().mainExtension()) {
			return core::make_shared<VXCFormat>();
		} else if (ext == VXTFormat::format().mainExtension()) {
			return core::make_shared<VXTFormat>();
		} else if (ext == VXLFormat::format().mainExtension() && desc.name == VXLFormat::format().name) {
			return core::make_shared<VXLFormat>();
		} else if (ext == AoSVXLFormat::format().mainExtension() && desc.name == AoSVXLFormat::format().name) {
			return core::make_shared<AoSVXLFormat>();
		} else if (ext == CSMFormat::formatNVM().mainExtension() || ext == CSMFormat::format().mainExtension()) {
			return core::make_shared<CSMFormat>();
		} else if (ext == BinVoxFormat::format().mainExtension()) {
			return core::make_shared<BinVoxFormat>();
		} else if (ext == QEFFormat::format().mainExtension()) {
			return core::make_shared<QEFFormat>();
		} else if (ext == QBCLFormat::format().mainExtension()) {
			return core::make_shared<QBCLFormat>();
		} else if (ext == OBJFormat::format().mainExtension()) {
			return core::make_shared<OBJFormat>();
		} else if (ext == STLFormat::format().mainExtension()) {
			return core::make_shared<STLFormat>();
		} else if (ext == QuakeBSPFormat::formatUFOAI().mainExtension() || ext == QuakeBSPFormat::formatQuake1().mainExtension()) {
			return core::make_shared<QuakeBSPFormat>();
		} else if (ext == MapFormat::format().mainExtension()) {
			return core::make_shared<MapFormat>();
		} else if (ext == PLYFormat::format().mainExtension()) {
			return core::make_shared<PLYFormat>();
		} else if (ext == FBXFormat::format().mainExtension()) {
			return core::make_shared<FBXFormat>();
		} else if (ext == Autodesk3DSFormat::format().mainExtension()) {
			return core::make_shared<Autodesk3DSFormat>();
		} else if (ext == MDLFormat::format().mainExtension()) {
			return core::make_shared<MDLFormat>();
		} else if (ext == MD2Format::format().mainExtension()) {
			return core::make_shared<MD2Format>();
		} else if (ext == SchematicFormat::format().mainExtension()) {
			return core::make_shared<SchematicFormat>();
		} else if (ext == VBXFormat::format().mainExtension()) {
			return core::make_shared<VBXFormat>();
		} else if (ext == XRawFormat::format().mainExtension()) {
			return core::make_shared<XRawFormat>();
		} else if (ext == V3AFormat::format().mainExtension()) {
			return core::make_shared<V3AFormat>();
		} else if (ext == PCubesFormat::format().mainExtension()) {
			return core::make_shared<PCubesFormat>();
		} else if (ext == CubzhFormat::format().mainExtension()) {
			return core::make_shared<CubzhFormat>();
		} else if (ext == CubzhB64Format::format().mainExtension()) {
			return core::make_shared<CubzhB64Format>();
		} else if (ext == AsepriteFormat::format().mainExtension()) {
			return core::make_shared<AsepriteFormat>();
		} else if (ext == ThingFormat::format().mainExtension()) {
			return core::make_shared<ThingFormat>();
		} else if (ext == io::format::png().mainExtension()) {
			return core::make_shared<PNGFormat>();
		} else if (GLTFFormat::format().matchesExtension(ext)) {
			return core::make_shared<GLTFFormat>();
		} else if (ext == SpriteStackFormat::format().mainExtension()) {
			return core::make_shared<SpriteStackFormat>();
		} else if (BenVoxelFormat::format().matchesExtension(ext)) {
			return core::make_shared<BenVoxelFormat>();
		} else {
			Log::warn("Unknown extension %s", ext.c_str());
		}
	}
	return {};
}

static uint32_t loadMagic(const core::String &filename, const io::ArchivePtr &archive) {
	core::ScopedPtr<io::SeekableReadStream> stream(archive->readStream(filename));
	if (!stream) {
		Log::warn("Failed to open file at %s", filename.c_str());
		return false;
	}
	const uint32_t magic = loadMagic(*stream);
	return magic;
}

image::ImagePtr loadScreenshot(const core::String &filename, const io::ArchivePtr &archive, const LoadContext &ctx) {
	core_trace_scoped(LoadVolumeScreenshot);
	const uint32_t magic = loadMagic(filename, archive);

	const io::FormatDescription *desc = io::getDescription(filename, magic, voxelLoad());
	if (desc == nullptr) {
		Log::warn("Format %s isn't supported for loading screenshots", filename.c_str());
		return image::ImagePtr();
	}
	if (!(desc->flags & VOX_FORMAT_FLAG_SCREENSHOT_EMBEDDED)) {
		Log::debug("Format %s doesn't have a screenshot embedded", desc->name.c_str());
		return image::ImagePtr();
	}
	const core::SharedPtr<Format> &f = getFormat(*desc, magic);
	if (f) {
		return f->loadScreenshot(filename, archive, ctx);
	}
	Log::error("Failed to load model screenshot from file %s - "
			   "unsupported file format",
			   filename.c_str());
	return image::ImagePtr();
}

bool importPalette(const core::String &filename, palette::Palette &palette) {
	if (io::isA(filename, palette::palettes())) {
		return palette.load(filename.c_str());
	}
	if (io::isA(filename, voxelformat::voxelLoad())) {
		const io::ArchivePtr &archive = io::openFilesystemArchive(io::filesystem());
		LoadContext loadCtx;
		if (voxelformat::loadPalette(filename, archive, palette, loadCtx) <= 0) {
			Log::warn("Failed to load palette from %s", filename.c_str());
			return false;
		}
		return true;
	}
	Log::warn("Given file is not supported as palette source: %s", filename.c_str());
	return false;
}

size_t loadPalette(const core::String &filename, const io::ArchivePtr &archive, palette::Palette &palette,
				   const LoadContext &ctx) {
	core_trace_scoped(LoadVolumePalette);
	const uint32_t magic = loadMagic(filename, archive);
	const io::FormatDescription *desc = io::getDescription(filename, magic, voxelLoad());
	if (desc == nullptr) {
		Log::warn("Format %s isn't supported", filename.c_str());
		return 0;
	}
	if (const core::SharedPtr<Format> &f = getFormat(*desc, magic)) {
		const size_t n = f->loadPalette(filename, archive, palette, ctx);
		palette.markDirty();
		return n;
	}
	Log::error("Failed to load model palette from file %s - "
			   "unsupported file format",
			   filename.c_str());
	return 0;
}

bool loadFormat(const io::FileDescription &fileDesc, const io::ArchivePtr &archive,
				scenegraph::SceneGraph &newSceneGraph, const LoadContext &ctx) {
	core_trace_scoped(LoadVolumeFormat);
	const uint32_t magic = loadMagic(fileDesc.name, archive);
	const io::FormatDescription *desc = io::getDescription(fileDesc, magic, voxelLoad());
	if (desc == nullptr) {
		return false;
	}
	const core::String &filename = fileDesc.name;
	const core::SharedPtr<Format> &f = getFormat(*desc, magic);
	if (f) {
		if (!f->load(filename, archive, newSceneGraph, ctx)) {
			Log::error("Error while loading %s", filename.c_str());
			newSceneGraph.clear();
		}
	} else {
		Log::error("Failed to load model file %s - unsupported "
				   "file format",
				   filename.c_str());
		return false;
	}
	const int models = (int)newSceneGraph.size(scenegraph::SceneGraphNodeType::Model);
	const int points = (int)newSceneGraph.size(scenegraph::SceneGraphNodeType::Point);
	if (models == 0 && points == 0) {
		Log::error("Failed to load model file %s. Scene graph "
				   "doesn't contain models.",
				   filename.c_str());
		return false;
	}
	Log::info("Load file %s with %i model nodes and %i point nodes", filename.c_str(), models, points);
	const core::String &ext = core::string::extractExtension(filename);
	if (!ext.empty()) {
		metric::count("load", 1, {{"type", ext.toLower()}});
	}
	return true;
}

bool isMeshFormat(const io::FormatDescription &desc) {
	return desc.flags & VOX_FORMAT_FLAG_MESH;
}

bool isPaletteEmbedded(const io::FormatDescription &desc) {
	return desc.flags & VOX_FORMAT_FLAG_PALETTE_EMBEDDED;
}

bool isRGBFormat(const io::FormatDescription &desc) {
	return desc.flags & VOX_FORMAT_FLAG_RGB;
}

bool isAnimationSupported(const io::FormatDescription &desc) {
	return desc.flags & VOX_FORMAT_FLAG_ANIMATION;
}

bool isMeshFormat(const core::String &filename, bool save) {
	const core::String &ext = core::string::extractExtension(filename);
	for (const io::FormatDescription *desc = save ? voxelformat::voxelSave() : voxelformat::voxelLoad(); desc->valid();
		 ++desc) {
		if (desc->matchesExtension(ext) && isMeshFormat(*desc)) {
			return true;
		}
	}

	return false;
}

bool isModelFormat(const core::String &filename) {
	const core::String &ext = core::string::extractExtension(filename);
	for (const io::FormatDescription *desc = voxelformat::voxelLoad(); desc->valid(); ++desc) {
		if (desc->matchesExtension(ext)) {
			return true;
		}
	}

	return false;
}

bool saveFormat(scenegraph::SceneGraph &sceneGraph, const core::String &filename, const io::FormatDescription *desc,
				const io::ArchivePtr &archive, const SaveContext &ctx) {
	if (sceneGraph.empty()) {
		Log::error("Failed to save model file %s - no volumes given", filename.c_str());
		return false;
	}
	const core::String &ext = core::string::extractExtension(filename);
	if (desc) {
		if (!desc->matchesExtension(ext)) {
			desc = nullptr;
		}
	}
	if (desc != nullptr) {
		core::SharedPtr<Format> f = getFormat(*desc, 0u);
		if (f) {
			if (f->save(sceneGraph, filename, archive, ctx)) {
				Log::debug("Saved file for format '%s' (ext: '%s')", desc->name.c_str(), ext.c_str());
				metric::count("save", 1, {{"type", ext.toLower()}});
				return true;
			}
			Log::error("Failed to save %s file", desc->name.c_str());
			return false;
		}
	}
	for (desc = voxelformat::voxelSave(); desc->valid(); ++desc) {
		if (!desc->matchesExtension(ext)) {
			continue;
		}
		core::SharedPtr<Format> f = getFormat(*desc, 0u);
		if (f) {
			if (f->save(sceneGraph, filename, archive, ctx)) {
				Log::debug("Saved file for format '%s' (ext: '%s')", desc->name.c_str(), ext.c_str());
				metric::count("save", 1, {{"type", ext.toLower()}});
				return true;
			}
			Log::error("Failed to save %s file", desc->name.c_str());
			return false;
		}
	}
	return false;
}

} // namespace voxelformat
