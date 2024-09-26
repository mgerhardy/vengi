/**
 * @file
 */

#include "VoxConvert.h"
#include "core/Enum.h"
#include "core/GameConfig.h"
#include "core/Log.h"
#include "core/ScopedPtr.h"
#include "core/StringUtil.h"
#include "core/TimeProvider.h"
#include "core/Var.h"
#include "core/collection/DynamicArray.h"
#include "core/collection/Set.h"
#include "core/collection/StringSet.h"
#include "core/concurrent/Concurrency.h"
#include "engine-git.h"
#include "image/Image.h"
#include "io/Archive.h"
#include "io/FileStream.h"
#include "io/Filesystem.h"
#include "io/FilesystemArchive.h"
#include "io/FormatDescription.h"
#include "io/Stream.h"
#include "io/ZipArchive.h"
#include "palette/Palette.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/SceneGraphNode.h"
#include "scenegraph/SceneGraphUtil.h"
#include "voxel/ChunkMesh.h"
#include "voxel/MaterialColor.h"
#include "voxel/RawVolume.h"
#include "voxel/Region.h"
#include "voxel/SurfaceExtractor.h"
#include "voxel/Voxel.h"
#include "voxelformat/Format.h"
#include "voxelformat/FormatConfig.h"
#include "voxelformat/VolumeFormat.h"
#include "voxelgenerator/LUAApi.h"
#include "voxelutil/ImageUtils.h"
#include "voxelutil/VolumeCropper.h"
#include "voxelutil/VolumeRescaler.h"
#include "voxelutil/VolumeResizer.h"
#include "voxelutil/VolumeRotator.h"
#include "voxelutil/VolumeSplitter.h"
#include "voxelutil/VolumeVisitor.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>

VoxConvert::VoxConvert(const io::FilesystemPtr &filesystem, const core::TimeProviderPtr &timeProvider)
	: Super(filesystem, timeProvider, core::cpus()) {
	init(ORGANISATION, "voxconvert");
	_wantCrashLogs = true;
}

app::AppState VoxConvert::onConstruct() {
	const app::AppState state = Super::onConstruct();
	registerArg("--crop").setDescription("Reduce the models to their real voxel sizes");
	registerArg("--json").setDescription(
		"Print the scene graph of the input file. Give full as argument to also get mesh details");
	registerArg("--export-models").setDescription("Export all the models of a scene into single files");
	registerArg("--export-palette").setDescription("Export the palette data into the given output file format");
	registerArg("--filter").setDescription("Model filter. For example '1-4,6'");
	registerArg("--filter-property").setDescription("Model filter by property. For example 'name:foo'");
	registerArg("--force").setShort("-f").setDescription("Overwrite existing files");
	registerArg("--input").setShort("-i").setDescription("Allow to specify input files").addFlag(ARGUMENT_FLAG_FILE);
	registerArg("--wildcard")
		.setShort("-w")
		.setDescription("Allow to specify input file filter if --input is a directory");
	registerArg("--merge").setShort("-m").setDescription("Merge models into one volume");
	registerArg("--mirror").setDescription("Mirror by the given axis (x, y or z)");
	registerArg("--output")
		.setShort("-o")
		.setDescription("Allow to specify the output file")
		.addFlag(ARGUMENT_FLAG_FILE);
	registerArg("--rotate")
		.setDescription(
			"Rotate by 90 degree at the given axis (x, y or z), specify e.g. x:180 to rotate around x by 180 degree.");
	registerArg("--resize").setDescription("Resize the volume by the given x (right), y (up) and z (back) values");
	registerArg("--scale").setShort("-s").setDescription("Scale model to 50% of its original size");
	registerArg("--script")
		.setDefaultValue("script.lua")
		.setDescription("Apply the given lua script to the output volume");
	registerArg("--scriptcolor")
		.setDefaultValue("1")
		.setDescription("Set the palette index that is given to the color script parameters of the main function");
	registerArg("--split").setDescription("Slices the models into pieces of the given size <x:y:z>");
	registerArg("--surface-only")
		.setDescription("Remove any non surface voxel. If you are meshing with this, you get also faces on the inner "
						"side of your mesh.");
	registerArg("--translate").setShort("-t").setDescription("Translate the models by x (right), y (up), z (back)");
	registerArg("--print-formats").setDescription("Print supported formats as json for easier parsing in other tools");

	voxelformat::FormatConfig::init();

	_mergeQuads = core::Var::getSafe(cfg::VoxformatMergequads);
	_reuseVertices = core::Var::getSafe(cfg::VoxformatReusevertices);
	_ambientOcclusion = core::Var::getSafe(cfg::VoxformatAmbientocclusion);
	_scale = core::Var::getSafe(cfg::VoxformatScale);
	_scaleX = core::Var::getSafe(cfg::VoxformatScaleX);
	_scaleY = core::Var::getSafe(cfg::VoxformatScaleY);
	_scaleZ = core::Var::getSafe(cfg::VoxformatScaleZ);
	_quads = core::Var::getSafe(cfg::VoxformatQuads);
	_withColor = core::Var::getSafe(cfg::VoxformatWithColor);
	_withTexCoords = core::Var::getSafe(cfg::VoxformatWithtexcoords);

	if (!filesystem()->registerPath(core::Path("scripts/"))) {
		Log::warn("Failed to register lua generator script path");
	}

	return state;
}

void VoxConvert::printUsageHeader() const {
	Super::printUsageHeader();
	Log::info("Git commit " GIT_COMMIT " - " GIT_COMMIT_DATE);
}

void VoxConvert::usage() const {
	Super::usage();
	Log::info("Load support:");
	for (const io::FormatDescription *desc = voxelformat::voxelLoad(); desc->valid(); ++desc) {
		for (const core::String &ext : desc->exts) {
			Log::info(" * %s (*.%s)", desc->name.c_str(), ext.c_str());
		}
	}
	Log::info("Save support:");
	for (const io::FormatDescription *desc = voxelformat::voxelSave(); desc->valid(); ++desc) {
		for (const core::String &ext : desc->exts) {
			Log::info(" * %s (*.%s)", desc->name.c_str(), ext.c_str());
		}
	}
	Log::info("Supported image formats:");
	for (const io::FormatDescription *desc = io::format::images(); desc->valid(); ++desc) {
		for (const core::String &ext : desc->exts) {
			Log::info(" * %s (*.%s)", desc->name.c_str(), ext.c_str());
		}
	}
	Log::info("Supported palette formats:");
	for (const io::FormatDescription *desc = io::format::palettes(); desc->valid(); ++desc) {
		for (const core::String &ext : desc->exts) {
			Log::info(" * %s (*.%s)", desc->name.c_str(), ext.c_str());
		}
	}
	Log::info("Built-in palettes:");
	for (int i = 0; i < lengthof(palette::Palette::builtIn); ++i) {
		Log::info(" * %s", palette::Palette::builtIn[i]);
	}
	Log::info("Links:");
	Log::info(" * Bug reports: https://github.com/vengi-voxel/vengi");
	Log::info(" * Twitter: https://twitter.com/MartinGerhardy");
	Log::info(" * Mastodon: https://mastodon.social/@mgerhardy");
	Log::info(" * Discord: https://vengi-voxel.de/discord");

	if (core::Var::getSafe(cfg::MetricFlavor)->strVal().empty()) {
		Log::info(
			"Please enable anonymous usage statistics. You can do this by setting the metric_flavor cvar to 'json'");
		Log::info("Example: '%s -set metric_flavor json --input xxx --output yyy'", fullAppname().c_str());
	}
}

static void printFormatDetails(const io::FormatDescription *desc, const core::StringMap<uint32_t> &flags = {}) {
	const io::FormatDescription *first = desc;
	for (; desc->valid(); ++desc) {
		if (desc != first) {
			Log::printf(",");
		}
		Log::printf("{");
		Log::printf("\"name\":\"%s\",", desc->name.c_str());
		Log::printf("\"extensions\":[");
		for (size_t i = 0; i < desc->exts.size(); ++i) {
			if (i > 0) {
				Log::printf(",");
			}
			Log::printf("\"%s\"", desc->exts[i].c_str());
		}
		Log::printf("]");
		for (const auto &entry : flags) {
			if (desc->flags & entry->second) {
				Log::printf(",\"%s\":true", entry->first.c_str());
			}
		}
		Log::printf("}");
	}
}

bool VoxConvert::slice(const scenegraph::SceneGraph &sceneGraph, const core::String &outfile) {
	const core::String &ext = core::string::extractExtension(outfile);
	const core::String &basePath = core::string::stripExtension(outfile);
	for (const auto &e : sceneGraph.nodes()) {
		const scenegraph::SceneGraphNode &node = e->value;
		if (!node.isModelNode()) {
			continue;
		}
		Log::info("Slice model %s", node.name().c_str());
		const voxel::RawVolume *volume = node.volume();
		core_assert(volume != nullptr);
		const voxel::Region &region = volume->region();
		const palette::Palette &palette = node.palette();
		for (int z = region.getLowerZ(); z <= region.getUpperZ(); ++z) {
			const core::String &filename = core::string::format("%s-%i.%s", basePath.c_str(), z, ext.c_str());
			image::Image image(filename);
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
				Log::error("Failed to load slice image %s", filename.c_str());
				return false;
			}
			if (!image::writeImage(image, filename)) {
				Log::error("Failed to write slice image %s", filename.c_str());
				return false;
			}
		}
	}
	Log::info("Sliced models into %s", outfile.c_str());
	return true;
}

app::AppState VoxConvert::onInit() {
	const app::AppState state = Super::onInit();
	if (state != app::AppState::Running) {
		return state;
	}

	if (_argc < 2) {
		_logLevelVar->setVal(SDL_LOG_PRIORITY_INFO);
		Log::init();
		usage();
		return app::AppState::InitFailure;
	}

	if (hasArg("--print-formats")) {
		Log::printf("{\"voxels\":[");
		printFormatDetails(voxelformat::voxelLoad(), {{"thumbnail_embedded", VOX_FORMAT_FLAG_SCREENSHOT_EMBEDDED},
													  {"palette_embedded", VOX_FORMAT_FLAG_PALETTE_EMBEDDED},
													  {"mesh", VOX_FORMAT_FLAG_MESH},
													  {"animation", VOX_FORMAT_FLAG_ANIMATION},
													  {"save", FORMAT_FLAG_SAVE}});
		Log::printf("],\"images\":[");
		printFormatDetails(io::format::images(), {{"save", FORMAT_FLAG_SAVE}});
		Log::printf("],\"palettes\":[");
		printFormatDetails(io::format::palettes(), {{"save", FORMAT_FLAG_SAVE}});
		Log::printf("]}\n");
		return state;
	}

	const bool hasScript = hasArg("--script");

	core::String infilesstr;
	core::DynamicArray<core::String> infiles;
	bool inputIsMesh = false;
	if (hasArg("--input")) {
		int argn = 0;
		for (;;) {
			core::String val = getArgVal("--input", "", &argn);
			if (val.empty()) {
				break;
			}
			io::normalizePath(val);
			infiles.push_back(val);
			if (voxelformat::isMeshFormat(val, false)) {
				inputIsMesh = true;
			}
			if (!infilesstr.empty()) {
				infilesstr += ", ";
			}
			infilesstr += val;
		}
	} else if (!hasScript) {
		Log::error("No input file was specified");
		return app::AppState::InitFailure;
	}

	core::String outfilesstr;
	core::DynamicArray<core::String> outfiles;
	bool outputIsMesh = false;
	if (hasArg("--output")) {
		int argn = 0;
		for (;;) {
			core::String val = getArgVal("--output", "", &argn);
			if (val.empty()) {
				break;
			}
			io::normalizePath(val);
			outfiles.push_back(val);
			if (voxelformat::isMeshFormat(val, false)) {
				outputIsMesh = true;
			}
			if (!outfilesstr.empty()) {
				outfilesstr += ", ";
			}
			outfilesstr += val;
		}
	}

	_mergeModels = hasArg("--merge");
	_scaleModels = hasArg("--scale");
	_mirrorModels = hasArg("--mirror");
	_rotateModels = hasArg("--rotate");
	_translateModels = hasArg("--translate");
	_exportPalette = hasArg("--export-palette");
	_exportModels = hasArg("--export-models");
	_cropModels = hasArg("--crop");
	_surfaceOnly = hasArg("--surface-only");
	_splitModels = hasArg("--split");
	_printSceneGraph = hasArg("--json");
	_resizeModels = hasArg("--resize");

	Log::info("Options");
	if (inputIsMesh || outputIsMesh) {
		Log::info("* mergeQuads:        - %s", _mergeQuads->strVal().c_str());
		Log::info("* reuseVertices:     - %s", _reuseVertices->strVal().c_str());
		Log::info("* ambientOcclusion:  - %s", _ambientOcclusion->strVal().c_str());
		Log::info("* scale:             - %s", _scale->strVal().c_str());
		Log::info("* scaleX:            - %s", _scaleX->strVal().c_str());
		Log::info("* scaleY:            - %s", _scaleY->strVal().c_str());
		Log::info("* scaleZ:            - %s", _scaleZ->strVal().c_str());
		Log::info("* quads:             - %s", _quads->strVal().c_str());
		Log::info("* withColor:         - %s", _withColor->strVal().c_str());
		Log::info("* withTexCoords:     - %s", _withTexCoords->strVal().c_str());
	}
	const core::VarPtr &paletteVar = core::Var::getSafe(cfg::VoxelPalette);
	if (!paletteVar->strVal().empty()) {
		Log::info("* palette:           - %s", paletteVar->strVal().c_str());
	}
	Log::info("* input files:       - %s", infilesstr.c_str());
	if (!outfilesstr.empty()) {
		Log::info("* output files:      - %s", outfilesstr.c_str());
	}
	core::String scriptParameters;
	if (hasScript) {
		scriptParameters = getArgVal("--script");
		if (scriptParameters.empty()) {
			Log::error("Missing script parameters");
		}
		Log::info("* script:            - %s", scriptParameters.c_str());
	}
	Log::info("* show scene graph:  - %s", (_printSceneGraph ? "true" : "false"));
	Log::info("* merge models:      - %s", (_mergeModels ? "true" : "false"));
	Log::info("* scale models:      - %s", (_scaleModels ? "true" : "false"));
	Log::info("* crop models:       - %s", (_cropModels ? "true" : "false"));
	Log::info("* surface only:      - %s", (_surfaceOnly ? "true" : "false"));
	Log::info("* split models:      - %s", (_splitModels ? "true" : "false"));
	Log::info("* mirror models:     - %s", (_mirrorModels ? "true" : "false"));
	Log::info("* translate models:  - %s", (_translateModels ? "true" : "false"));
	Log::info("* rotate models:     - %s", (_rotateModels ? "true" : "false"));
	Log::info("* export palette:    - %s", (_exportPalette ? "true" : "false"));
	Log::info("* export models:     - %s", (_exportModels ? "true" : "false"));
	Log::info("* resize models:     - %s", (_resizeModels ? "true" : "false"));

	if (core::Var::getSafe(cfg::MetricFlavor)->strVal().empty()) {
		Log::info(
			"Please enable anonymous usage statistics. You can do this by setting the metric_flavor cvar to 'json'");
		Log::info("Example: '%s -set metric_flavor json --input xxx --output yyy'", fullAppname().c_str());
	}

	if (!outfiles.empty()) {
		if (!hasArg("--force")) {
			for (const core::String &outfile : outfiles) {
				const bool outfileExists = filesystem()->open(outfile)->exists();
				if (outfileExists) {
					Log::error("Given output file '%s' already exists", outfile.c_str());
					return app::AppState::InitFailure;
				}
			}
		}
	} else if (!_exportModels && !_printSceneGraph) {
		Log::error("No output specified");
		return app::AppState::InitFailure;
	}

	const io::ArchivePtr &fsArchive = io::openFilesystemArchive(filesystem());
	scenegraph::SceneGraph sceneGraph;
	for (const core::String &infile : infiles) {
		if (shouldQuit()) {
			break;
		}
		if (filesystem()->sysIsReadableDir(infile)) {
			core::DynamicArray<io::FilesystemEntry> entities;
			filesystem()->list(infile, entities, getArgVal("--wildcard", ""));
			Log::info("Found %i entries in dir %s", (int)entities.size(), infile.c_str());
			int success = 0;
			for (const io::FilesystemEntry &entry : entities) {
				if (shouldQuit()) {
					break;
				}
				if (entry.type != io::FilesystemEntry::Type::file) {
					continue;
				}
				const core::String fullpath = core::string::path(infile, entry.name);
				if (handleInputFile(fullpath, fsArchive, sceneGraph, entities.size() > 1)) {
					++success;
				}
			}
			if (success == 0) {
				Log::error("Could not find a valid input file in directory %s", infile.c_str());
				return app::AppState::InitFailure;
			}
		} else if (io::isZipArchive(infile)) {
			io::FileStream archiveStream(filesystem()->open(infile, io::FileMode::SysRead));
			io::ArchivePtr archive = io::openZipArchive(&archiveStream);
			if (!archive) {
				Log::error("Failed to open archive %s", infile.c_str());
				return app::AppState::InitFailure;
			}

			const core::String filter = getArgVal("--wildcard", "");
			for (const auto &entry : archive->files()) {
				if (shouldQuit()) {
					break;
				}
				if (!entry.isFile()) {
					continue;
				}
				if (!io::isA(entry.name, voxelformat::voxelLoad())) {
					continue;
				}
				if (!filter.empty()) {
					if (!core::string::fileMatchesMultiple(entry.name.c_str(), filter.c_str())) {
						Log::debug("Entity %s doesn't match filter %s", entry.name.c_str(), filter.c_str());
						continue;
					}
				}
				const core::Path &fullPath = filesystem()->homeWritePath(entry.fullPath.str());
				if (!handleInputFile(fullPath.str(), archive, sceneGraph, archive->files().size() > 1)) {
					Log::error("Failed to handle input file %s", fullPath.c_str());
				}
			}
		} else {
			if (!handleInputFile(infile, fsArchive, sceneGraph, infiles.size() > 1)) {
				return app::AppState::InitFailure;
			}
		}
	}
	if (!scriptParameters.empty() && sceneGraph.empty()) {
		scenegraph::SceneGraphNode node(scenegraph::SceneGraphNodeType::Model);
		const voxel::Region region(0, 63);
		node.setVolume(new voxel::RawVolume(region), true);
		node.setName("Script generated");
		sceneGraph.emplace(core::move(node));
	}

	if (sceneGraph.empty()) {
		if (_exportPalette) {
			return state;
		}
		Log::error("No valid input found in the scenegraph to operate on.");
		return app::AppState::InitFailure;
	}

	const bool applyFilter = hasArg("--filter");
	if (applyFilter) {
		if (infiles.size() == 1u) {
			filterModels(sceneGraph);
		} else {
			Log::warn("Don't apply model filters for multiple input files");
		}
	}

	const bool applyFilterProperty = hasArg("--filter-property");
	if (applyFilterProperty) {
		if (infiles.size() == 1u) {
			const core::String &property = getArgVal("--filter-property");
			core::String key = property;
			core::String value;
			const size_t colonPos = property.find(":");
			if (colonPos != core::String::npos) {
				key = property.substr(0, colonPos);
				value = property.substr(colonPos + 1);
			}
			filterModelsByProperty(sceneGraph, key, value);
		} else {
			Log::warn("Don't apply model property filters for multiple input files");
		}
	}

	if (_exportModels) {
		if (infiles.size() > 1) {
			Log::warn("The format and path of the first input file is used for exporting all models");
		}
		for (const core::String &outfile : outfiles) {
			io::FilePtr outputFile = filesystem()->open(outfile, io::FileMode::SysWrite);
			if (!outputFile->validHandle()) {
				Log::error("Could not open target file: %s", outfile.c_str());
				return app::AppState::InitFailure;
			}
			exportModelsIntoSingleObjects(sceneGraph, infiles[0], outputFile ? outputFile->extension() : "");
		}
		return state;
	}

	if (_mergeModels) {
		Log::info("Merge models");
		const scenegraph::SceneGraph::MergedVolumePalette &merged = sceneGraph.merge();
		if (merged.first == nullptr) {
			Log::error("Failed to merge models");
			return app::AppState::InitFailure;
		}
		sceneGraph.clear();
		scenegraph::SceneGraphNode node;
		node.setPalette(merged.second);
		node.setVolume(merged.first, true);
		node.setName(infilesstr);
		sceneGraph.emplace(core::move(node));
	}

	if (_scaleModels) {
		scale(sceneGraph);
	}

	if (_resizeModels) {
		resize(getArgIvec3("--resize"), sceneGraph);
	}

	if (_mirrorModels) {
		mirror(getArgVal("--mirror"), sceneGraph);
	}

	if (_rotateModels) {
		rotate(getArgVal("--rotate"), sceneGraph);
	}

	if (_translateModels) {
		translate(getArgIvec3("--translate"), sceneGraph);
	}

	if (!scriptParameters.empty()) {
		const core::String &color = getArgVal("--scriptcolor");
		script(scriptParameters, sceneGraph, color.toInt());
	}

	if (_cropModels) {
		crop(sceneGraph);
	}

	if (_surfaceOnly) {
		removeNonSurfaceVoxels(sceneGraph);
	}

	if (_splitModels) {
		split(getArgIvec3("--split"), sceneGraph);
	}

	for (const core::String &outfile : outfiles) {
		if (_exportPalette || (!io::isA(outfile, voxelformat::voxelSave()) && io::isA(outfile, io::format::palettes()))) {
			// if the given format is a palette only format (some voxel formats might have the same
			// extension - so we check that here)
			const palette::Palette &palette = sceneGraph.mergePalettes(false);
			if (!palette.save(outfile.c_str())) {
				Log::error("Failed to save palette to %s", outfile.c_str());
				return app::AppState::InitFailure;
			}
			Log::info("Saved palette with %i colors to %s", palette.colorCount(), outfile.c_str());
		} else {
			Log::debug("Save %i models", (int)sceneGraph.size());
			voxelformat::SaveContext saveCtx;
			const io::ArchivePtr &archive = io::openFilesystemArchive(filesystem());
			if (!voxelformat::saveFormat(sceneGraph, outfile, nullptr, archive, saveCtx)) {
				Log::error("Failed to write to output file '%s'", outfile.c_str());
				return app::AppState::InitFailure;
			}
			Log::info("Wrote output file %s", outfile.c_str());
		}
	}
	return state;
}

core::String VoxConvert::getFilenameForModelName(const core::String &inputfile, const core::String &modelName,
												 const core::String &outExt, int id, bool uniqueNames) {
	const core::String &ext = outExt.empty() ? core::string::extractExtension(inputfile) : outExt;
	core::String name;
	if (modelName.empty()) {
		name = core::string::format("model-%i.%s", id, ext.c_str());
	} else if (uniqueNames) {
		name = core::string::format("%s.%s", modelName.c_str(), ext.c_str());
	} else {
		name = core::string::format("%s-%i.%s", modelName.c_str(), id, ext.c_str());
	}
	return core::string::path(core::string::extractDir(inputfile), core::string::sanitizeFilename(name));
}

static void printProgress(const char *name, int cur, int max) {
	// Log::info("%s: %i/%i", name, cur, max);
}

bool VoxConvert::handleInputFile(const core::String &infile, const io::ArchivePtr &archive,
								 scenegraph::SceneGraph &sceneGraph, bool multipleInputs) {
	Log::info("-- current input file: %s", infile.c_str());
	core::ScopedPtr<io::SeekableReadStream> stream(archive->readStream(infile));
	if (!stream) {
		Log::error("Given input file '%s' does not exist", infile.c_str());
		_exitCode = 127;
		return false;
	}
	scenegraph::SceneGraph newSceneGraph;
	voxelformat::LoadContext loadCtx;
	loadCtx.monitor = printProgress;
	io::FileDescription fileDesc;
	fileDesc.set(infile);
	if (!voxelformat::loadFormat(fileDesc, archive, newSceneGraph, loadCtx)) {
		return false;
	}

	int parent = sceneGraph.root().id();
	if (multipleInputs) {
		scenegraph::SceneGraphNode groupNode(scenegraph::SceneGraphNodeType::Group);
		groupNode.setName(core::string::extractFilename(infile));
		parent = sceneGraph.emplace(core::move(groupNode), parent);
	}
	scenegraph::addSceneGraphNodes(sceneGraph, newSceneGraph, parent);
	if (_printSceneGraph) {
		sceneGraphJson(sceneGraph, getArgVal("--json", "") == "full");
	}

	return true;
}

static bool hasUniqueModelNames(const scenegraph::SceneGraph &sceneGraph) {
	core::StringSet names;
	for (const auto &entry : sceneGraph.nodes()) {
		const scenegraph::SceneGraphNode &node = entry->value;
		if (!node.isModelNode()) {
			continue;
		}
		if (!names.insert(node.name())) {
			return false;
		}
	}
	return true;
}

void VoxConvert::exportModelsIntoSingleObjects(scenegraph::SceneGraph &sceneGraph, const core::String &inputfile,
											   const core::String &ext) {
	Log::info("Export models into single objects");
	int id = 0;
	voxelformat::SaveContext saveCtx;
	const auto &nodes = sceneGraph.nodes();
	const bool uniqueNames = hasUniqueModelNames(sceneGraph);
	for (auto entry : nodes) {
		const scenegraph::SceneGraphNode &node = entry->value;
		if (!node.isModelNode()) {
			continue;
		}
		scenegraph::SceneGraph newSceneGraph;
		scenegraph::SceneGraphNode newNode;
		scenegraph::copyNode(node, newNode, false);
		newSceneGraph.emplace(core::move(newNode));
		const core::String &filename = getFilenameForModelName(inputfile, node.name(), ext, id, uniqueNames);
		const io::ArchivePtr &archive = io::openFilesystemArchive(filesystem());
		if (voxelformat::saveFormat(newSceneGraph, filename, nullptr, archive, saveCtx)) {
			Log::info(" .. %s", filename.c_str());
		} else {
			Log::error(" .. %s", filename.c_str());
		}
		++id;
	}
}

glm::ivec3 VoxConvert::getArgIvec3(const core::String &name) {
	const core::String &arguments = getArgVal(name);
	glm::ivec3 t(0);
	SDL_sscanf(arguments.c_str(), "%i:%i:%i", &t.x, &t.y, &t.z);
	return t;
}

void VoxConvert::split(const glm::ivec3 &size, scenegraph::SceneGraph &sceneGraph) {
	Log::info("split volumes at %i:%i:%i", size.x, size.y, size.z);
	const scenegraph::SceneGraph::MergedVolumePalette &merged = sceneGraph.merge();
	sceneGraph.clear();
	core::DynamicArray<voxel::RawVolume *> rawVolumes;
	voxelutil::splitVolume(merged.first, size, rawVolumes);
	delete merged.first;
	for (voxel::RawVolume *v : rawVolumes) {
		scenegraph::SceneGraphNode node;
		node.setVolume(v, true);
		node.setPalette(merged.second);
		sceneGraph.emplace(core::move(node));
	}
}

VoxConvert::NodeStats VoxConvert::sceneGraphJsonNode_r(const scenegraph::SceneGraph &sceneGraph, int nodeId, bool printMeshDetails) const {
	const scenegraph::SceneGraphNode &node = sceneGraph.node(nodeId);

	const scenegraph::SceneGraphNodeType type = node.type();

	Log::printf("{");
	Log::printf("\"id\": %i,", nodeId);
	Log::printf("\"parent\": %i,", node.parent());
	Log::printf("\"name\": \"%s\",", node.name().c_str());
	Log::printf("\"type\": \"%s\",", scenegraph::SceneGraphNodeTypeStr[core::enumVal(type)]);
	const glm::vec3 &pivot = node.pivot();
	Log::printf("\"pivot\": \"%f:%f:%f\"", pivot.x, pivot.y, pivot.z);
	NodeStats stats;
	if (type == scenegraph::SceneGraphNodeType::Model) {
		const voxel::RawVolume *v = node.volume();
		const voxel::Region &region = node.region();
		Log::printf(",\"volume\": {");
		Log::printf("\"region\": {");
		Log::printf("\"mins\": \"%i:%i:%i\",", region.getLowerX(), region.getLowerY(),
				  region.getLowerZ());
		Log::printf("\"maxs\": \"%i:%i:%i\",", region.getUpperX(), region.getUpperY(),
				  region.getUpperZ());
		Log::printf("\"size\": \"%i:%i:%i\"", region.getWidthInVoxels(), region.getHeightInVoxels(),
				  region.getDepthInVoxels());
		Log::printf("},");
		if (v) {
			voxelutil::visitVolume(*v, [&](int, int, int, const voxel::Voxel &) { ++stats.voxels; });
		}
		Log::printf("\"voxels\": %i", stats.voxels);
		Log::printf("}");
	} else if (type == scenegraph::SceneGraphNodeType::Camera) {
		const scenegraph::SceneGraphNodeCamera &cameraNode = scenegraph::toCameraNode(node);
		Log::printf(",\"camera\": {");
		Log::printf("\"field_of_view\": %i,", cameraNode.fieldOfView());
		Log::printf("\"nearplane\": %f,", cameraNode.nearPlane());
		Log::printf("\"farplane\": %f,", cameraNode.farPlane());
		Log::printf("\"mode\": \"%s\"", cameraNode.isOrthographic() ? "ortho" : "perspective");
		Log::printf("}");
	}
	if (!node.properties().empty()) {
		Log::printf(",\"properties\": {");
		auto piter = node.properties().begin();
		for (size_t i = 0; i < node.properties().size(); ++i) {
			const auto &entry = *piter;
			Log::printf("\"%s\": \"%s\"", entry->key.c_str(), entry->value.c_str());
			if (i + 1 < node.properties().size()) {
				Log::printf(",");
			}
			++piter;
		}
		Log::printf("}");
	}
	Log::printf(",\"animations\": [");
	for (size_t a = 0; a < sceneGraph.animations().size(); ++a) {
		Log::printf("{");
		Log::printf("\"name\": \"%s\",", sceneGraph.animations()[a].c_str());
		Log::printf("\"keyframes\": [");
		for (size_t i = 0; i < node.keyFrames().size(); ++i) {
			const scenegraph::SceneGraphKeyFrame &kf = node.keyFrames()[i];
			Log::printf("{");
			Log::printf("\"id\": %i,", kf.frameIdx);
			Log::printf("\"long_rotation\": %s,", kf.longRotation ? "true" : "false");
			Log::printf("\"interpolation\": \"%s\",",
					scenegraph::InterpolationTypeStr[core::enumVal(kf.interpolation)]);
			Log::printf("\"transform\": {");
			const scenegraph::SceneGraphTransform &transform = kf.transform();
			const glm::vec3 &tr = transform.worldTranslation();
			Log::printf("\"world_translation\": {");
			Log::printf("\"x\": %f,", tr.x);
			Log::printf("\"y\": %f,", tr.y);
			Log::printf("\"z\": %f", tr.z);
			Log::printf("},");
			const glm::vec3 &ltr = transform.localTranslation();
			Log::printf("\"local_translation\": {");
			Log::printf("\"x\": %f,", ltr.x);
			Log::printf("\"y\": %f,", ltr.y);
			Log::printf("\"z\": %f", ltr.z);
			Log::printf("},");
			const glm::quat &rt = transform.worldOrientation();
			const glm::vec3 &rtEuler = glm::degrees(glm::eulerAngles(rt));
			Log::printf("\"world_orientation\": {");
			Log::printf("\"x\": %f,", rt.x);
			Log::printf("\"y\": %f,", rt.y);
			Log::printf("\"z\": %f,", rt.z);
			Log::printf("\"w\": %f", rt.w);
			Log::printf("},");
			Log::printf("\"world_euler\": {");
			Log::printf("\"x\": %f,", rtEuler.x);
			Log::printf("\"y\": %f,", rtEuler.y);
			Log::printf("\"z\": %f", rtEuler.z);
			Log::printf("},");
			const glm::quat &lrt = transform.localOrientation();
			const glm::vec3 &lrtEuler = glm::degrees(glm::eulerAngles(lrt));
			Log::printf("\"local_orientation\": {");
			Log::printf("\"x\": %f,", lrt.x);
			Log::printf("\"y\": %f,", lrt.y);
			Log::printf("\"z\": %f,", lrt.z);
			Log::printf("\"w\": %f", lrt.w);
			Log::printf("},");
			Log::printf("\"local_euler\": {");
			Log::printf("\"x\": %f,", lrtEuler.x);
			Log::printf("\"y\": %f,", lrtEuler.y);
			Log::printf("\"z\": %f", lrtEuler.z);
			Log::printf("},");
			const glm::vec3 &sc = transform.worldScale();
			Log::printf("\"world_scale\": {");
			Log::printf("\"x\": %f,", sc.x);
			Log::printf("\"y\": %f,", sc.y);
			Log::printf("\"z\": %f", sc.z);
			Log::printf("},");
			const glm::vec3 &lsc = transform.localScale();
			Log::printf("\"local_scale\": {");
			Log::printf("\"x\": %f,", lsc.x);
			Log::printf("\"y\": %f,", lsc.y);
			Log::printf("\"z\": %f", lsc.z);
			Log::printf("}");
			Log::printf("}"); // transform
			Log::printf("}"); // keyframe
			if (i + 1 < node.keyFrames().size()) {
				Log::printf(",");
			}
		}
		Log::printf("]"); // keyframes
		Log::printf("}"); // animation
		if (a + 1 < sceneGraph.animations().size()) {
			Log::printf(",");
		}
	}
	Log::printf("]"); // animations

	if (printMeshDetails && node.isModelNode()) {
		const bool mergeQuads = core::Var::getSafe(cfg::VoxformatMergequads)->boolVal();
		const bool reuseVertices = core::Var::getSafe(cfg::VoxformatReusevertices)->boolVal();
		const bool ambientOcclusion = core::Var::getSafe(cfg::VoxformatAmbientocclusion)->boolVal();
		const voxel::SurfaceExtractionType meshType =
			(voxel::SurfaceExtractionType)core::Var::getSafe(cfg::VoxelMeshMode)->intVal();
		voxel::ChunkMesh mesh;
		voxel::SurfaceExtractionContext ctx =
			voxel::createContext(meshType, node.volume(), node.region(), node.palette(), mesh, {0, 0, 0}, mergeQuads,
								 reuseVertices, ambientOcclusion);

		voxel::extractSurface(ctx);
		const size_t vertices = mesh.mesh[0].getNoOfVertices() + mesh.mesh[1].getNoOfVertices();
		const size_t indices = mesh.mesh[0].getNoOfIndices() + mesh.mesh[1].getNoOfIndices();
		Log::printf(",\"mesh\": {");
		Log::printf("\"vertices\": %i,", (int)vertices);
		Log::printf("\"indices\": %i", (int)indices);
		Log::printf("}");
		stats.vertices += (int)vertices;
		stats.indices += (int)indices;
	}
	if (!node.children().empty()) {
		Log::printf(",\"children\": [");
		for (size_t i = 0; i < node.children().size(); ++i) {
			const int children = node.children()[i];
			stats += sceneGraphJsonNode_r(sceneGraph, children, printMeshDetails);
			if (i + 1 < node.children().size()) {
				Log::printf(",");
			}
		}
		Log::printf("]");
	}
	Log::printf("}");
	return stats;
}

void VoxConvert::sceneGraphJson(const scenegraph::SceneGraph &sceneGraph, bool printMeshDetails) const {
	Log::printf("{");
	Log::printf("\"root\": ");
	NodeStats stats = sceneGraphJsonNode_r(sceneGraph, sceneGraph.root().id(), printMeshDetails);
	Log::printf(",");
	Log::printf("\"stats\": {");
	Log::printf("\"voxel_count\": %i", stats.voxels);
	if (printMeshDetails) {
		Log::printf(",\"vertex_count\": %i,", stats.vertices);
		Log::printf("\"index_count\": %i", stats.indices);
	}
	Log::printf("}"); // stats
	Log::printf("}");
}

void VoxConvert::crop(scenegraph::SceneGraph &sceneGraph) {
	Log::info("Crop volumes");
	for (auto iter = sceneGraph.beginModel(); iter != sceneGraph.end(); ++iter) {
		scenegraph::SceneGraphNode &node = *iter;
		node.setVolume(voxelutil::cropVolume(node.volume()), true);
	}
}

void VoxConvert::removeNonSurfaceVoxels(scenegraph::SceneGraph &sceneGraph) {
	Log::info("Remove non-surface voxels");
	for (auto iter = sceneGraph.beginModel(); iter != sceneGraph.end(); ++iter) {
		scenegraph::SceneGraphNode &node = *iter;
		core::DynamicArray<glm::ivec3> filled;
		voxelutil::visitUndergroundVolume(*node.volume(), [&filled](int x, int y, int z, const voxel::Voxel &voxel) {
			filled.emplace_back(x, y, z);
		});
		for (const glm::ivec3 &pos : filled) {
			node.volume()->setVoxel(pos, voxel::Voxel());
		}
	}
}

void VoxConvert::script(const core::String &scriptParameters, scenegraph::SceneGraph &sceneGraph, uint8_t color) {
	voxelgenerator::LUAApi script(_filesystem);
	if (!script.init()) {
		Log::warn("Failed to initialize the script bindings");
	} else {
		core::DynamicArray<core::String> tokens;
		core::string::splitString(scriptParameters, tokens);
		const core::String &luaScript = script.load(tokens[0]);
		if (luaScript.empty()) {
			Log::error("Failed to load %s", tokens[0].c_str());
		} else {
			const voxel::Voxel voxel = voxel::createVoxel(voxel::VoxelType::Generic, color);
			core::DynamicArray<voxelgenerator::LUAParameterDescription> argsInfo;
			if (!script.argumentInfo(luaScript, argsInfo)) {
				Log::warn("Failed to get argument details");
			}
			core::DynamicArray<core::String> args(tokens.size() - 1);
			for (size_t i = 1; i < tokens.size(); ++i) {
				args[i - 1] = tokens[i];
			}
			Log::info("Execute script %s", tokens[0].c_str());
			core::DynamicArray<int> nodes;
			for (auto iter = sceneGraph.beginModel(); iter != sceneGraph.end(); ++iter) {
				nodes.push_back((*iter).id());
			}
			for (int nodeId : nodes) {
				const scenegraph::SceneGraphNode &node = sceneGraph.node(nodeId);
				Log::debug("execute for node: %i", nodeId);
				if (!script.exec(luaScript, sceneGraph, node.id(), node.region(), voxel, args)) {
					break;
				}
				while (script.scriptStillRunning()) {
					script.update(0.0);
				}
			}
		}
	}

	script.shutdown();
}

void VoxConvert::scale(scenegraph::SceneGraph &sceneGraph) {
	Log::info("Scale models");
	for (auto iter = sceneGraph.beginModel(); iter != sceneGraph.end(); ++iter) {
		scenegraph::SceneGraphNode &node = *iter;
		const voxel::Region srcRegion = node.region();
		const glm::ivec3 &targetDimensionsHalf = (srcRegion.getDimensionsInVoxels() / 2) - 1;
		const voxel::Region destRegion(srcRegion.getLowerCorner(), srcRegion.getLowerCorner() + targetDimensionsHalf);
		if (destRegion.isValid()) {
			voxel::RawVolume *destVolume = new voxel::RawVolume(destRegion);
			voxelutil::scaleDown(*node.volume(), node.palette(), *destVolume);
			node.setVolume(destVolume, true);
		}
	}
}

void VoxConvert::resize(const glm::ivec3 &size, scenegraph::SceneGraph &sceneGraph) {
	Log::info("Resize models");
	for (auto iter = sceneGraph.beginModel(); iter != sceneGraph.end(); ++iter) {
		scenegraph::SceneGraphNode &node = *iter;
		voxel::RawVolume *v = voxelutil::resize(node.volume(), size);
		if (v == nullptr) {
			Log::warn("Failed to resize volume");
			continue;
		}
		node.setVolume(v, true);
	}
}

void VoxConvert::filterModels(scenegraph::SceneGraph &sceneGraph) {
	const core::String &filter = getArgVal("--filter");
	if (filter.empty()) {
		Log::warn("No filter specified");
		return;
	}

	core::Set<int> models;
	core::DynamicArray<core::String> tokens;
	core::string::splitString(filter, tokens, ",");
	for (const core::String &token : tokens) {
		if (token.contains("-")) {
			const int start = token.toInt();
			const size_t index = token.find("-");
			const core::String &endString = token.substr(index + 1);
			const int end = endString.toInt();
			for (int modelIndex = start; modelIndex <= end; ++modelIndex) {
				models.insert(modelIndex);
			}
		} else {
			const int modelIndex = token.toInt();
			models.insert(modelIndex);
		}
	}
	auto iter = sceneGraph.beginModel();
	core::Set<int> removeNodes;
	for (int i = 0; iter != sceneGraph.end(); ++i, ++iter) {
		if (!models.has(i)) {
			removeNodes.insert((*iter).id());
			Log::debug("Remove model %i - not part of the filter expression", i);
		}
	}
	for (const auto &entry : removeNodes) {
		sceneGraph.removeNode(entry->key, false);
	}
	Log::info("Filtered models: %i", (int)models.size());
}

void VoxConvert::filterModelsByProperty(scenegraph::SceneGraph &sceneGraph, const core::String &property,
										const core::String &value) {
	if (property.empty()) {
		Log::warn("No property specified to filter");
		return;
	}

	core::Set<int> removeNodes;
	for (auto iter : sceneGraph.nodes()) {
		const scenegraph::SceneGraphNode &node = iter->value;
		if (!node.isModelNode()) {
			continue;
		}
		const core::String &pkey = node.property(property);
		if (value.empty() && !pkey.empty()) {
			continue;
		}
		if (pkey == value) {
			continue;
		}
		Log::debug("Remove model %i - not part of the filter expression", node.id());
		removeNodes.insert(node.id());
	}
	for (const auto &entry : removeNodes) {
		sceneGraph.removeNode(entry->key, false);
	}
	Log::info("Filtered models: %i", (int)sceneGraph.size(scenegraph::SceneGraphNodeType::Model));
}

void VoxConvert::mirror(const core::String &axisStr, scenegraph::SceneGraph &sceneGraph) {
	const math::Axis axis = math::toAxis(axisStr);
	if (axis == math::Axis::None) {
		return;
	}
	Log::info("Mirror on axis %c", axisStr[0]);
	for (auto iter = sceneGraph.beginModel(); iter != sceneGraph.end(); ++iter) {
		scenegraph::SceneGraphNode &node = *iter;
		node.setVolume(voxelutil::mirrorAxis(node.volume(), axis), true);
	}
}

void VoxConvert::rotate(const core::String &axisStr, scenegraph::SceneGraph &sceneGraph) {
	const math::Axis axis = math::toAxis(axisStr);
	if (axis == math::Axis::None) {
		return;
	}
	float degree = 90.0f;
	if (axisStr.contains(":")) {
		degree = glm::mod(axisStr.substr(2).toFloat(), 360.0f);
	}
	if (degree <= 1.0f) {
		Log::warn("Don't rotate on axis %c by %f degree", axisStr[0], degree);
		return;
	}
	Log::info("Rotate on axis %c by %f degree", axisStr[0], degree);
	for (auto iter = sceneGraph.beginModel(); iter != sceneGraph.end(); ++iter) {
		scenegraph::SceneGraphNode &node = *iter;
		glm::vec3 rotVec{0.0f};
		rotVec[math::getIndexForAxis(axis)] = degree;
		node.setVolume(voxelutil::rotateVolume(node.volume(), node.palette(), rotVec, glm::vec3(0.5f)), true);
	}
}

void VoxConvert::translate(const glm::ivec3 &pos, scenegraph::SceneGraph &sceneGraph) {
	Log::info("Translate by %i:%i:%i", pos.x, pos.y, pos.z);
	for (auto iter = sceneGraph.beginModel(); iter != sceneGraph.end(); ++iter) {
		scenegraph::SceneGraphNode &node = *iter;
		if (voxel::RawVolume *v = node.volume()) {
			v->translate(pos);
		}
	}
}

int main(int argc, char *argv[]) {
	const io::FilesystemPtr &filesystem = core::make_shared<io::Filesystem>();
	const core::TimeProviderPtr &timeProvider = core::make_shared<core::TimeProvider>();
	VoxConvert app(filesystem, timeProvider);
	return app.startMainLoop(argc, argv);
}
