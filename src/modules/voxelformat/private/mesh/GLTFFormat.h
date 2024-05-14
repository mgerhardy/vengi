/**
 * @file
 */

#pragma once

#include "MeshFormat.h"
#include "core/Pair.h"
#include "core/collection/StringMap.h"
#include "palette/Palette.h"

namespace tinygltf {
class Model;
class Node;
struct Mesh;
struct Scene;
struct Material;
struct Primitive;
struct Accessor;
struct Animation;
struct AnimationChannel;
struct TextureInfo;
} // namespace tinygltf

namespace scenegraph {
class SceneGraphTransform;
}
namespace voxelformat {

/**
 * @brief GL Transmission Format
 * https://raw.githubusercontent.com/KhronosGroup/glTF/main/specification/2.0/figures/gltfOverview-2.0.0b.png
 *
 * @li Viewer including animations: https://sandbox.babylonjs.com/
 * @li GLTF-Validator: https://github.khronos.org/glTF-Validator/
 * @li GLTF Extensions: https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos
 *
 * @ingroup Formats
 */
class GLTFFormat : public MeshFormat {
private:
	// extensions
	/**
	 * https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_emissive_strength
	 */
	void save_KHR_materials_emissive_strength(const palette::Material &material,
											  tinygltf::Material &gltfMaterial, tinygltf::Model &gltfModel) const;
	void load_KHR_materials_emissive_strength(palette::Material &material,
											  const tinygltf::Material &gltfMaterial) const;
	/**
	 * https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_ior
	 */
	void save_KHR_materials_ior(const palette::Material &material, tinygltf::Material &gltfMaterial, tinygltf::Model &gltfModel) const;
	void load_KHR_materials_ior(palette::Material &material, const tinygltf::Material &gltfMaterial) const;
	/**
	 * https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_volume
	 */
	void save_KHR_materials_volume(const palette::Material &material, const core::RGBA &color, tinygltf::Material &gltfMaterial, tinygltf::Model &gltfModel) const;
	/**
	 * https://kcoley.github.io/glTF/extensions/2.0/Khronos/KHR_materials_pbrSpecularGlossiness
	 */
	void save_KHR_materials_pbrSpecularGlossiness(const palette::Material &material, const core::RGBA &color,
												  tinygltf::Material &gltfMaterial, tinygltf::Model &gltfModel) const;
	void load_KHR_materials_pbrSpecularGlossiness(palette::Material &material,
												  const tinygltf::Material &gltfMaterial) const;
	/**
	 * https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_specular
	 */
	void save_KHR_materials_specular(const palette::Material &material, const core::RGBA &color,
									 tinygltf::Material &gltfMaterial, tinygltf::Model &gltfModel) const;
	void load_KHR_materials_specular(palette::Material &material, const tinygltf::Material &gltfMaterial) const;

	// exporting
	struct Bounds {
		uint32_t maxIndex = 0u;
		uint32_t minIndex = 0u;
		uint32_t ni = 0u;
		uint32_t nv = 0u;
		glm::vec3 maxVertex{0.0f};
		glm::vec3 minVertex{0.0f};
	};
	using Stack = core::DynamicArray<core::Pair<int, int>>;
	using MaterialMap = core::Map<uint64_t, core::Array<int, palette::PaletteMaxColors>>;
	void saveGltfNode(core::Map<int, int> &nodeMapping, tinygltf::Model &gltfModel, tinygltf::Scene &gltfScene,
					  const scenegraph::SceneGraphNode &graphNode, Stack &stack,
					  const scenegraph::SceneGraph &sceneGraph, const glm::vec3 &scale, bool exportAnimations);
	uint32_t writeBuffer(const voxel::Mesh *mesh, uint8_t idx, io::SeekableWriteStream &os, bool withColor,
						 bool withTexCoords, bool colorAsFloat, bool exportNormals, bool applyTransform,
						 const glm::vec3 &pivotOffset, const palette::Palette &palette, Bounds &bounds);
	int saveEmissiveTexture(tinygltf::Model &gltfModel, const palette::Palette &palette) const;
	int saveTexture(tinygltf::Model &gltfModel, const palette::Palette &palette) const;
	void generateMaterials(bool withTexCoords, tinygltf::Model &gltfModel, MaterialMap &paletteMaterialIndices,
						   const scenegraph::SceneGraphNode &node, const palette::Palette &palette,
						   int &texcoordIndex) const;
	bool savePrimitivesPerMaterial(uint8_t idx, const glm::vec3 &pivotOffset, tinygltf::Model &gltfModel,
								   tinygltf::Mesh &gltfMesh, const voxel::Mesh *mesh, const palette::Palette &palette,
								   bool withColor, bool withTexCoords, bool colorAsFloat, bool exportNormals,
								   bool applyTransform, int texcoordIndex, const MaterialMap &paletteMaterialIndices);

	void saveAnimation(int targetNode, tinygltf::Model &m, const scenegraph::SceneGraphNode &node,
					   tinygltf::Animation &gltfAnimation);

	// importing (voxelization)
	struct GltfVertex {
		glm::vec3 pos{0.0f};
		glm::vec2 uv{0.0f};
		image::TextureWrap wrapS = image::TextureWrap::Repeat;
		image::TextureWrap wrapT = image::TextureWrap::Repeat;
		core::RGBA color{0};
		core::String texture;
	};
	struct GltfMaterialData {
		core::String diffuseTexture;
		core::String texCoordAttribute;
		image::TextureWrap wrapS = image::TextureWrap::Repeat;
		image::TextureWrap wrapT = image::TextureWrap::Repeat;
		core::RGBA baseColor{255, 255, 255, 255};
	};
	void loadTexture(const core::String &filename, core::StringMap<image::ImagePtr> &textures,
					 const tinygltf::Model &gltfModel, GltfMaterialData &materialData,
					 const tinygltf::TextureInfo &gltfTextureInfo, int textureIndex) const;
	bool loadMaterial(const core::String &filename, core::StringMap<image::ImagePtr> &textures,
					  const tinygltf::Model &gltfModel, const tinygltf::Primitive &gltfPrimitive,
					  GltfMaterialData &materialData, palette::Material &material) const;
	bool loadAttributes(const core::String &filename, core::StringMap<image::ImagePtr> &textures,
						const tinygltf::Model &gltfModel, const tinygltf::Primitive &gltfPrimitive,
						core::DynamicArray<GltfVertex> &vertices, palette::Material &material) const;

	bool loadAnimationChannel(const tinygltf::Model &gltfModel, const tinygltf::Animation &gltfAnimation,
							  const tinygltf::AnimationChannel &gltfAnimChannel,
							  scenegraph::SceneGraphNode &node) const;
	bool loadAnimations(scenegraph::SceneGraph &sceneGraph, const tinygltf::Model &model, int gltfNodeIdx,
						scenegraph::SceneGraphNode &node) const;
	bool loadNode_r(const core::String &filename, scenegraph::SceneGraph &sceneGraph,
					core::StringMap<image::ImagePtr> &textures, const tinygltf::Model &gltfModel, int gltfNodeIdx,
					int parentNodeId) const;
	bool loadIndices(const tinygltf::Model &model, const tinygltf::Primitive &gltfPrimitive,
					 core::DynamicArray<uint32_t> &indices, size_t indicesOffset) const;
	scenegraph::SceneGraphTransform loadTransform(const tinygltf::Node &gltfNode) const;
	size_t accessorSize(const tinygltf::Accessor &gltfAccessor) const;
	const tinygltf::Accessor *getAccessor(const tinygltf::Model &gltfModel, int id) const;

	bool voxelizeGroups(const core::String &filename, const io::ArchivePtr &archive,
						scenegraph::SceneGraph &sceneGraph, const LoadContext &ctx) override;

public:
	bool saveMeshes(const core::Map<int, int> &meshIdxNodeMap, const scenegraph::SceneGraph &sceneGraph,
					const Meshes &meshes, const core::String &filename, const io::ArchivePtr &archive,
					const glm::vec3 &scale, bool quad, bool withColor, bool withTexCoords) override;
};

} // namespace voxelformat
