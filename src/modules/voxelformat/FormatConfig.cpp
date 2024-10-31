/**
 * @file
 */

#include "FormatConfig.h"
#include "app/I18N.h"
#include "core/Color.h"
#include "core/ConfigVar.h"
#include "core/StringUtil.h"
#include "core/Var.h"
#include "palette/Palette.h"
#include "voxel/SurfaceExtractor.h"
#include "voxelformat/private/image/PNGFormat.h"
#include "voxelformat/private/mesh/MeshFormat.h"

namespace voxelformat {

static bool colorReductionValidator(const core::String &value) {
	return core::Color::toColorReductionType(value.c_str()) != core::Color::ColorReductionType::Max;
}

bool FormatConfig::init() {
	core::Var::get(cfg::CoreColorReduction,
				   core::Color::toColorReductionTypeString(core::Color::ColorReductionType::MedianCut),
				   _("Controls the algorithm that is used to perform the color reduction"), colorReductionValidator);
	core::Var::get(cfg::VoxformatMergequads, "true", core::CV_NOPERSIST, _("Merge similar quads to optimize the mesh"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxelMeshMode, core::string::toString((int)voxel::SurfaceExtractionType::Binary),
				   core::CV_SHADER, _("0 = cubes, 1 = marching cubes, 2 = binary mesher"),
				   core::Var::minMaxValidator<(int)voxel::SurfaceExtractionType::Cubic,
											  (int)voxel::SurfaceExtractionType::Max - 1>);
	core::Var::get(cfg::VoxformatReusevertices, "true", core::CV_NOPERSIST,
				   _("Reuse vertices or always create new ones"), core::Var::boolValidator);
	core::Var::get(cfg::VoxformatRGBWeightedAverage, "true", core::CV_NOPERSIST,
				   _("If multiple triangles contribute to the same voxel the color values are averaged based on their "
					 "area contribution - otherwise only the biggest triangle counts"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxformatAmbientocclusion, "false", core::CV_NOPERSIST,
				   _("Extra vertices for ambient occlusion"), core::Var::boolValidator);
	core::Var::get(cfg::VoxformatRGBFlattenFactor, "0", core::CV_NOPERSIST,
				   _("The RGB color flatten factor for importing color and mesh formats"), [](const core::String &var) {
					   const int type = var.toInt();
					   return type >= 0 && type <= 255;
				   });
	core::Var::get(cfg::VoxformatSaveVisibleOnly, "false", core::CV_NOPERSIST, _("Save only visible nodes"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxformatScale, "1.0", core::CV_NOPERSIST,
				   _("Scale the vertices for voxelization on all axis by the given factor"));
	core::Var::get(cfg::VoxformatScaleX, "1.0", core::CV_NOPERSIST,
				   _("Scale the vertices for voxelization X axis by the given factor"));
	core::Var::get(cfg::VoxformatScaleY, "1.0", core::CV_NOPERSIST,
				   _("Scale the vertices for voxelization Y axis by the given factor"));
	core::Var::get(cfg::VoxformatScaleZ, "1.0", core::CV_NOPERSIST,
				   _("Scale the vertices for voxelization Z axis by the given factor"));
	core::Var::get(cfg::VoxformatQuads, "true", core::CV_NOPERSIST,
				   _("Export as quads. If this false, triangles will be used."), core::Var::boolValidator);
	core::Var::get(cfg::VoxformatWithColor, "true", core::CV_NOPERSIST, _("Export with vertex colors"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxformatWithNormals, "false", core::CV_NOPERSIST,
				   _("Export smoothed normals for cubic meshes"), core::Var::boolValidator);
	core::Var::get(cfg::VoxformatColorAsFloat, "true", core::CV_NOPERSIST,
				   _("Export with vertex colors as float values (if vertex colors are exported)"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxformatWithtexcoords, "true", core::CV_NOPERSIST,
				   _("Export with uv coordinates of the palette image"), core::Var::boolValidator);
	core::Var::get(cfg::VoxformatTransform, "true", core::CV_NOPERSIST,
				   _("Apply the scene graph transform to mesh exports"), core::Var::boolValidator);
	core::Var::get(cfg::VoxformatOptimize, "false", core::CV_NOPERSIST, _("Apply mesh optimization steps to meshes"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxformatFillHollow, "true", core::CV_NOPERSIST,
				   _("Fill the hollows when voxelizing a mesh format"), core::Var::boolValidator);
	core::Var::get(cfg::VoxformatVoxelizeMode, MeshFormat::VoxelizeMode::HighQuality, core::CV_NOPERSIST,
				   _("0 = high quality, 1 = faster and less memory"), core::Var::minMaxValidator<0, 1>);
	core::Var::get(cfg::VoxformatQBTPaletteMode, "true", core::CV_NOPERSIST,
				   _("Use palette mode in qubicle qbt export"), core::Var::boolValidator);
	core::Var::get(cfg::VoxformatQBTMergeCompounds, "false", core::CV_NOPERSIST, _("Merge compounds on load"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxelPalette, palette::Palette::getDefaultPaletteName(),
				   _("This is the NAME part of palette-<NAME>.png or absolute png file to use (1x256)"));
	core::Var::get(cfg::VoxformatMerge, "false", core::CV_NOPERSIST, _("Merge all objects into one"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxformatEmptyPaletteIndex, "-1", core::CV_NOPERSIST,
				   _("The index of the empty color in the palette"), [](const core::String &var) {
					   const int type = var.toInt();
					   return type >= -1 && type <= 255;
				   });
	core::Var::get(cfg::VoxformatVOXCreateGroups, "true", core::CV_NOPERSIST, _("Merge compounds on load"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxformatVOXCreateLayers, "true", core::CV_NOPERSIST, _("Merge compounds on load"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxformatQBSaveLeftHanded, "true", core::CV_NOPERSIST,
				   _("Toggle between left and right handed"), core::Var::boolValidator);
	core::Var::get(cfg::VoxformatQBSaveCompressed, "true", core::CV_NOPERSIST, _("Save RLE compressed"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxelCreatePalette, "true", core::CV_NOPERSIST, _("Create own palette from textures or colors or remap the existing palette colors to a new palette"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxformatPointCloudSize, "1", core::CV_NOPERSIST,
				   _("Specify the side length for the voxels when loading a point cloud"));
	core::Var::get(cfg::VoxFormatGLTF_KHR_materials_pbrSpecularGlossiness, "true", core::CV_NOPERSIST,
				   _("Apply KHR_materials_pbrSpecularGlossiness when saving into the gltf format"),
				   core::Var::boolValidator);
	core::Var::get(cfg::VoxFormatGLTF_KHR_materials_specular, "false", core::CV_NOPERSIST,
				   _("Apply KHR_materials_specular when saving into the gltf format"), core::Var::boolValidator);
	core::Var::get(cfg::VoxFormatWithMaterials, "true", core::CV_NOPERSIST,
				   _("Try to export material properties if the formats support it"), core::Var::boolValidator);
	core::Var::get(cfg::VoxformatImageVolumeMaxDepth, "1", core::CV_NOPERSIST,
				   _("The maximum depth of the volume when importing an image as volume"),
				   core::Var::minMaxValidator<1, 255>);
	core::Var::get(cfg::VoxformatImageVolumeBothSides, "true", core::CV_NOPERSIST,
				   _("Import the image as volume for both sides"), core::Var::boolValidator);
	core::Var::get(cfg::VoxformatTexturePath, "", core::CV_NOPERSIST,
				   _("Register an additional search path for texture lookups"));
	core::Var::get(cfg::VoxformatImageImportType, PNGFormat::ImportType::Plane, core::CV_NOPERSIST,
				   _("0 = plane, 1 = heightmap, 2 = volume"),
				   core::Var::minMaxValidator<PNGFormat::ImportType::Plane, PNGFormat::ImportType::Volume>);
	static_assert(PNGFormat::ImportType::Plane == 0, "Plane must be 0");
	static_assert(PNGFormat::ImportType::Volume == 2, "Volume must be 2");

	core::Var::get(cfg::PalformatRGB6Bit, "false", core::CV_NOPERSIST,
				   _("Use 6 bit color values for the palette (0-63) - used e.g. in C&C pal files"),
				   core::Var::boolValidator);

	return true;
}

} // namespace voxelformat
