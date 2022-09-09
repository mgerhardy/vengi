/**
 * @file
 */

#include "VoxFormat.h"
#include "core/ArrayLength.h"
#include "core/Assert.h"
#include "core/Color.h"
#include "core/Common.h"
#include "core/FourCC.h"
#include "core/Log.h"
#include "core/StandardLib.h"
#include "core/StringUtil.h"
#include "core/collection/DynamicArray.h"
#include "math/Math.h"
#include "voxel/MaterialColor.h"
#include "voxel/RawVolume.h"
#include "voxel/Voxel.h"
#include "voxelformat/SceneGraph.h"
#include "voxelformat/SceneGraphNode.h"
#include "voxelutil/VolumeVisitor.h"

#define OGT_VOX_IMPLEMENTATION
#include "external/ogt_vox.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

namespace voxelformat {

static void *_ogt_alloc(size_t size) {
	return core_malloc(size);
}

static void _ogt_free(void *mem) {
	core_free(mem);
}

static const ogt_vox_transform ogt_identity_transform {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

VoxFormat::VoxFormat() {
	ogt_vox_set_memory_allocator(_ogt_alloc, _ogt_free);
}

size_t VoxFormat::loadPalette(const core::String &filename, io::SeekableReadStream &stream, voxel::Palette &palette) {
	const size_t size = stream.size();
	uint8_t *buffer = (uint8_t *)core_malloc(size);
	if (stream.read(buffer, size) == -1) {
		core_free(buffer);
		return 0;
	}
	const ogt_vox_scene *scene = ogt_vox_read_scene_with_flags(buffer, size, 0);
	core_free(buffer);
	if (scene == nullptr) {
		Log::error("Could not load scene %s", filename.c_str());
		return 0;
	}
	palette.colorCount = lengthof(scene->palette.color);
	for (int i = 0; i < palette.colorCount; ++i) {
		const ogt_vox_rgba &c = scene->palette.color[i];
		const ogt_vox_matl &matl = scene->materials.matl[i];
		palette.colors[i] = core::Color::getRGBA(c.r, c.g, c.b, c.a);
		if (matl.type == ogt_matl_type::ogt_matl_type_emit) {
			palette.glowColors[i] = palette.colors[i];
		}
	}
	ogt_vox_destroy_scene(scene);
	return palette.size();
}

/**
 * @brief Calculate the scene graph object transformation. Used for the voxel and the AABB of the volume.
 *
 * @param mat The world space model matrix (rotation and translation) for the chunk
 * @param pos The position inside the untransformed chunk (local position)
 * @param pivot The pivot to do the rotation around. This is the @code chunk_size - 1 + 0.5 @endcode. Please
 * note that the @c w component must be @c 0.0
 * @return glm::vec4 The transformed world position
 */
static inline glm::vec4 calcTransform(const glm::mat4x4 &mat, const glm::ivec3 &pos, const glm::vec4 &pivot) {
	return glm::floor(mat * (glm::vec4((float)pos.x + 0.5f, (float)pos.y + 0.5f, (float)pos.z + 0.5f, 1.0f) - pivot));
}

static bool loadKeyFrames(voxelformat::SceneGraph &sceneGraph, voxelformat::SceneGraphNode& node, const ogt_vox_keyframe_transform* transformKeyframes, uint32_t numKeyframes) {
	SceneGraphKeyFrames kf;
	Log::debug("Load %d keyframes", numKeyframes);
	kf.reserve(numKeyframes);
	for (uint32_t keyFrameIdx = 0; keyFrameIdx < numKeyframes; ++keyFrameIdx) {
		const ogt_vox_keyframe_transform& transform_keyframe = transformKeyframes[keyFrameIdx];
		const ogt_vox_transform& keyframeTransform = transform_keyframe.transform;
		const glm::vec4 ogtKeyFrameCol0(keyframeTransform.m00, keyframeTransform.m01, keyframeTransform.m02, keyframeTransform.m03);
		const glm::vec4 ogtKeyFrameCol1(keyframeTransform.m10, keyframeTransform.m11, keyframeTransform.m12, keyframeTransform.m13);
		const glm::vec4 ogtKeyFrameCol2(keyframeTransform.m20, keyframeTransform.m21, keyframeTransform.m22, keyframeTransform.m23);
		const glm::vec4 ogtKeyFrameCol3(keyframeTransform.m30, keyframeTransform.m31, keyframeTransform.m32, keyframeTransform.m33);
		const glm::mat4 ogtKeyFrameMat = glm::mat4{ogtKeyFrameCol0, ogtKeyFrameCol1, ogtKeyFrameCol2, ogtKeyFrameCol3};
		SceneGraphKeyFrame sceneGraphKeyFrame;
		sceneGraphKeyFrame.frameIdx = transform_keyframe.frame_index;
		sceneGraphKeyFrame.interpolation = InterpolationType::Linear;
		sceneGraphKeyFrame.longRotation = false;
		sceneGraphKeyFrame.transform().setWorldMatrix(ogtKeyFrameMat);
		kf.push_back(sceneGraphKeyFrame);
	}
	return node.setKeyFrames(kf);
}

bool VoxFormat::addInstance(const ogt_vox_scene *scene, uint32_t ogt_instanceIdx, SceneGraph &sceneGraph, int parent, const glm::mat4 &zUpMat, const voxel::Palette &palette, bool groupHidden) {
	const ogt_vox_instance& ogtInstance = scene->instances[ogt_instanceIdx];
	const ogt_vox_transform& ogtTransform = ogtInstance.transform;
	const glm::vec4 ogtCol0(ogtTransform.m00, ogtTransform.m01, ogtTransform.m02, ogtTransform.m03);
	const glm::vec4 ogtCol1(ogtTransform.m10, ogtTransform.m11, ogtTransform.m12, ogtTransform.m13);
	const glm::vec4 ogtCol2(ogtTransform.m20, ogtTransform.m21, ogtTransform.m22, ogtTransform.m23);
	const glm::vec4 ogtCol3(ogtTransform.m30, ogtTransform.m31, ogtTransform.m32, ogtTransform.m33);
	const glm::mat4 ogtMat = glm::mat4{ogtCol0, ogtCol1, ogtCol2, ogtCol3};
	const ogt_vox_model *ogtModel = scene->models[ogtInstance.model_index];
	const uint8_t *ogtVoxels = ogtModel->voxel_data;
	const uint8_t *ogtVoxel = ogtVoxels;
	const glm::ivec3 maxs(ogtModel->size_x - 1, ogtModel->size_y - 1, ogtModel->size_z - 1);
	const glm::vec4 pivot(glm::floor((float)ogtModel->size_x / 2.0f), glm::floor((float)ogtModel->size_y / 2.0f), glm::floor((float)ogtModel->size_z / 2.0f), 0.0f);
	const glm::ivec3& transformedMins = calcTransform(ogtMat, glm::ivec3(0), pivot);
	const glm::ivec3& transformedMaxs = calcTransform(ogtMat, maxs, pivot);
	const glm::ivec3& zUpMins = calcTransform(zUpMat, transformedMins, glm::ivec4(0));
	const glm::ivec3& zUpMaxs = calcTransform(zUpMat, transformedMaxs, glm::ivec4(0));
	voxel::Region region(glm::min(zUpMins, zUpMaxs), glm::max(zUpMins, zUpMaxs));
	const glm::ivec3 shift = region.getLowerCorner();
	region.shift(-shift);
	voxel::RawVolume *v = new voxel::RawVolume(region);
	SceneGraphTransform transform;
	transform.setWorldTranslation(shift);

	for (uint32_t k = 0; k < ogtModel->size_z; ++k) {
		for (uint32_t j = 0; j < ogtModel->size_y; ++j) {
			for (uint32_t i = 0; i < ogtModel->size_x; ++i, ++ogtVoxel) {
				if (ogtVoxel[0] == 0) {
					continue;
				}
				const voxel::Voxel voxel = voxel::createVoxel(voxel::VoxelType::Generic, ogtVoxel[0]);
				const glm::ivec3 &pos = calcTransform(ogtMat, glm::ivec3(i, j, k), pivot);
				const glm::ivec3 &poszUp = calcTransform(zUpMat, pos, glm::ivec4(0));
				const glm::ivec3 &regionPos = poszUp - shift;
				v->setVoxel(regionPos, voxel);
			}
		}
	}

	SceneGraphNode node(SceneGraphNodeType::Model);
	const char *name = ogtInstance.name;
	if (name == nullptr) {
		const ogt_vox_layer &layer = scene->layers[ogtInstance.layer_index];
		name = layer.name;
		core::RGBA col;
		col.r = layer.color.r;
		col.g = layer.color.g;
		col.b = layer.color.b;
		col.a = layer.color.a;
		node.setColor(col);
		if (name == nullptr) {
			name = "";
		}
	}
	loadKeyFrames(sceneGraph, node, ogtInstance.transform_anim.keyframes, ogtInstance.transform_anim.num_keyframes);
	// TODO: we are overriding the keyframe data here
	node.setTransform(0, transform);
	node.setName(name);
	node.setVisible(!ogtInstance.hidden && !groupHidden);
	node.setVolume(v, true);
	node.setPalette(palette);
	return sceneGraph.emplace(core::move(node), parent) != -1;
}

bool VoxFormat::addGroup(const ogt_vox_scene *scene, uint32_t ogt_parentGroupIdx, SceneGraph &sceneGraph, int parent, const glm::mat4 &zUpMat, core::Set<uint32_t> &addedInstances, const voxel::Palette &palette) {
	const ogt_vox_group &ogt_group = scene->groups[ogt_parentGroupIdx];
	bool hidden = ogt_group.hidden;
	const char *name = "Group";
	const uint32_t layerIdx = ogt_group.layer_index;
	SceneGraphNode node(SceneGraphNodeType::Group);
	if (layerIdx < scene->num_layers) {
		const ogt_vox_layer &layer = scene->layers[layerIdx];
		hidden |= layer.hidden;
		if (layer.name != nullptr) {
			name = layer.name;
		}
		core::RGBA color;
		color.r = layer.color.r;
		color.g = layer.color.g;
		color.b = layer.color.b;
		color.a = layer.color.a;
		node.setColor(color);
	}
	loadKeyFrames(sceneGraph, node, ogt_group.transform_anim.keyframes, ogt_group.transform_anim.num_keyframes);
	node.setName(name);
	node.setVisible(!hidden);
	const int groupId = sceneGraph.emplace(core::move(node), parent);
	if (groupId == -1) {
		Log::error("Failed to add group node to the scene graph");
		return false;
	}

	for (uint32_t n = 0; n < scene->num_instances; ++n) {
		const ogt_vox_instance &ogtInstance = scene->instances[n];
		if (ogtInstance.group_index != ogt_parentGroupIdx) {
			continue;
		}
		if (!addedInstances.insert(n)) {
			continue;
		}
		if (!addInstance(scene, n, sceneGraph, groupId, zUpMat, palette, hidden)) {
			return false;
		}
	}

	for (uint32_t groupIdx = 0; groupIdx < scene->num_groups; ++groupIdx) {
		const ogt_vox_group &group = scene->groups[groupIdx];
		Log::debug("group %u with parent: %u (searching for %u)", groupIdx, group.parent_group_index, ogt_parentGroupIdx);
		if (group.parent_group_index != ogt_parentGroupIdx) {
			continue;
		}
		Log::debug("Found matching group (%u) with scene graph parent: %i", groupIdx, groupId);
		if (!addGroup(scene, groupIdx, sceneGraph, groupId, zUpMat, addedInstances, palette)) {
			return false;
		}
	}

	return groupId;
}

bool VoxFormat::loadGroupsPalette(const core::String &filename, io::SeekableReadStream &stream, SceneGraph &sceneGraph, voxel::Palette &palette) {
	const size_t size = stream.size();
	uint8_t *buffer = (uint8_t *)core_malloc(size);
	if (stream.read(buffer, size) == -1) {
		core_free(buffer);
		return false;
	}
	const uint32_t ogt_vox_flags = k_read_scene_flags_groups | k_read_scene_flags_keyframes | k_read_scene_flags_keep_empty_models_instances;
	const ogt_vox_scene *scene = ogt_vox_read_scene_with_flags(buffer, size, ogt_vox_flags);
	core_free(buffer);
	if (scene == nullptr) {
		Log::error("Could not load scene %s", filename.c_str());
		return false;
	}

	palette.colorCount = lengthof(scene->palette.color);
	for (int i = 0; i < palette.colorCount; ++i) {
		const ogt_vox_rgba color = scene->palette.color[i];
		palette.colors[i] = core::Color::getRGBA(color.r, color.g, color.b, color.a);
		const ogt_vox_matl &matl = scene->materials.matl[i];
		if (matl.type == ogt_matl_type_emit) {
			palette.glowColors[i] = palette.colors[i];
		}
	}
	// rotation matrix to convert into our coordinate system (mv has z pointing upwards)
	const glm::mat4 zUpMat = glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	core::Set<uint32_t> addedInstances;

	if (scene->num_groups > 0) {
		for (uint32_t i = 0; i < scene->num_groups; ++i) {
			const ogt_vox_group &group = scene->groups[i];
			// find the main group nodes
			if (group.parent_group_index != k_invalid_group_index) {
				continue;
			}
			Log::debug("Add root group %u/%u", i, scene->num_groups);
			if (!addGroup(scene, i, sceneGraph, sceneGraph.root().id(), zUpMat, addedInstances, palette)) {
				ogt_vox_destroy_scene(scene);
				return false;
			}
		}
	}
	for (uint32_t n = 0; n < scene->num_instances; ++n) {
		if (addedInstances.has(n)) {
			continue;
		}
		if (!addInstance(scene, n, sceneGraph, sceneGraph.root().id(), zUpMat, palette)) {
			ogt_vox_destroy_scene(scene);
			return false;
		}
	}

	for (uint32_t n = 0; n < scene->num_cameras; ++n) {
		const ogt_vox_cam& c = scene->cameras[n];
		const glm::vec3 target(c.focus[0], c.focus[1], c.focus[2]);
		const glm::quat quat(glm::vec3(c.angle[0], c.angle[1], c.angle[2]));
		const float distance = (float)c.radius;
		const glm::vec3& forward = glm::conjugate(quat) * glm::forward;
		const glm::vec3& backward = -forward;
		const glm::vec3& newPosition = target + backward * distance;
		const glm::mat4& orientation = glm::mat4_cast(quat);
		const glm::mat4& viewMatrix = glm::translate(orientation, -newPosition);
		// if (c.mode == ogt_cam_mode_perspective) {
		//  const float fieldOfView = glm::radians((float)c.fov);
		// }

		{
			SceneGraphNodeCamera camNode;
			camNode.setName(core::String::format("Camera %u", c.camera_id));
			SceneGraphTransform transform;
			transform.setWorldMatrix(viewMatrix);
			const KeyFrameIndex keyFrameIdx = 0;
			camNode.setTransform(keyFrameIdx, transform);
			camNode.setFieldOfView(c.fov);
			camNode.setFarPlane((float)c.radius);
			camNode.setProperty("frustum", core::String::format("%f", c.frustum)); // TODO:
			if (c.mode == ogt_cam_mode_perspective) {
				camNode.setPerspective();
			} else if (c.mode == ogt_cam_mode_orthographic) {
				camNode.setOrthographic();
			}
			sceneGraph.emplace(core::move(camNode), sceneGraph.root().id());
		}
	}

	ogt_vox_destroy_scene(scene);
	return true;
}

int VoxFormat::findClosestPaletteIndex(const voxel::Palette &palette) {
	// we have to find a replacement for the first palette entry - as this is used
	// as the empty voxel in magicavoxel
	core::DynamicArray<glm::vec4> materialColors;
	palette.toVec4f(materialColors);
	const glm::vec4 first = materialColors[0];
	materialColors.erase(materialColors.begin());
	return core::Color::getClosestMatch(first, materialColors) + 1;
}

bool VoxFormat::saveGroups(const SceneGraph &sceneGraph, const core::String &filename, io::SeekableWriteStream &stream) {
	SceneGraph newSceneGraph;
	splitVolumes(sceneGraph, newSceneGraph, glm::ivec3(256));

	const size_t modelCount = newSceneGraph.size();
	if (modelCount == 0) {
		Log::error("No model nodes found in scene graph");
		return false;
	}

	ogt_vox_group default_group;
	core_memset(&default_group, 0, sizeof(default_group));
	default_group.hidden = false;
	default_group.layer_index = 0;
	default_group.parent_group_index = k_invalid_group_index;
	default_group.transform = ogt_identity_transform;

	const voxel::Palette &palette = sceneGraph.firstPalette();
	const int replacement = findClosestPaletteIndex(palette);
	core::Buffer<ogt_vox_model> models(modelCount);
	core::Buffer<ogt_vox_layer> layers(modelCount);
	core::Buffer<ogt_vox_instance> instances(modelCount);
	core::Buffer<const ogt_vox_model *> modelPtr(modelCount);
	core::Array<ogt_vox_keyframe_transform, 4096> keyframeTransforms;
	int mdlIdx = 0;
	int transformKeyFrameIdx = 0;
	for (const SceneGraphNode &node : newSceneGraph) {
		const voxel::Region region = node.region();
		ogt_vox_model &model = models[mdlIdx];
		modelPtr[mdlIdx] = &model;
		// flip y and z here
		model.size_x = region.getWidthInVoxels();
		model.size_y = region.getDepthInVoxels();
		model.size_z = region.getHeightInVoxels();
		const int voxelSize = (int)(model.size_x * model.size_y * model.size_z);
		uint8_t *dataptr = (uint8_t*)core_malloc(voxelSize);
		model.voxel_data = dataptr;
		voxelutil::visitVolume(*node.volume(), [&] (int, int, int, const voxel::Voxel& voxel) {
			if (voxel.getColor() == 0 && !isAir(voxel.getMaterial())) {
				*dataptr++ = replacement;
			} else {
				*dataptr++ = voxel.getColor();
			}
		}, voxelutil::VisitAll(), voxelutil::VisitorOrder::YZmX);

		ogt_vox_layer &layer = layers[mdlIdx];
		layer.name = node.name().c_str();
		layer.hidden = !node.visible();
		const core::RGBA layerRGBA = node.color();
		layer.color.r = layerRGBA.r;
		layer.color.g = layerRGBA.g;
		layer.color.b = layerRGBA.b;
		layer.color.a = layerRGBA.a;

		ogt_vox_instance &instance = instances[mdlIdx];
		instance.group_index = 0;
		instance.model_index = mdlIdx;
		instance.layer_index = mdlIdx;
		instance.name = node.name().c_str();
		instance.hidden = !node.visible();
		const glm::vec3 &mins = region.getLowerCornerf();
		const glm::vec3 &maxs = region.getUpperCornerf();
		const glm::vec3 width = maxs - mins + 1.0f;

		instance.transform_anim.num_keyframes = node.keyFrames().size();
		instance.transform_anim.keyframes = instance.transform_anim.num_keyframes ? &keyframeTransforms[transformKeyFrameIdx] : nullptr;
		for (const SceneGraphKeyFrame& kf : node.keyFrames()) {
			ogt_vox_keyframe_transform kft;
			core_memset(&kft, 0, sizeof(kft));
			kft.frame_index = kf.frameIdx;
			kft.transform = ogt_identity_transform;
			// y and z are flipped here
			const glm::vec3 kftransform = mins + kf.transform().worldTranslation() + width / 2.0f;
			kft.transform.m30 = -glm::floor(kftransform.x + 0.5f);
			kft.transform.m31 = kftransform.z;
			kft.transform.m32 = kftransform.y;
			// TODO: apply rotation - but rotations are not interpolated - they must be aligned here somehow...
			keyframeTransforms[transformKeyFrameIdx++] = kft;
		}

		++mdlIdx;
	}

	ogt_vox_scene output_scene;
	core_memset(&output_scene, 0, sizeof(output_scene));
	output_scene.groups = &default_group;
	output_scene.num_groups = 1;
	output_scene.instances = &instances[0];
	output_scene.num_instances = instances.size();
	output_scene.layers = &layers[0];
	output_scene.num_layers = layers.size();
	const ogt_vox_model **modelsPtr = modelPtr.data();
	output_scene.models = modelsPtr;
	output_scene.num_models = modelPtr.size();
	core_memset(&output_scene.materials, 0, sizeof(output_scene.materials));

	size_t output_cameras = newSceneGraph.size(SceneGraphNodeType::Camera);
	core::Buffer<ogt_vox_cam> cameras(output_cameras);
	int camIdx = 0;
	for (auto iter = newSceneGraph.begin(SceneGraphNodeType::Camera); iter != newSceneGraph.end(); ++iter) {
		const SceneGraphNodeCamera &camera = toCameraNode(*iter);
		const SceneGraphTransform &transform = camera.transform(0);
		ogt_vox_cam &cam = cameras[camIdx];
		cam.camera_id = camIdx;
		const glm::vec3 &euler = glm::eulerAngles(transform.worldOrientation());
		cam.angle[0] = euler[0];
		cam.angle[1] = euler[1];
		cam.angle[2] = euler[2];
		const glm::vec3 &pos = transform.worldTranslation();
		cam.focus[0] = pos[0];
		cam.focus[1] = pos[1];
		cam.focus[2] = pos[2];
		cam.mode = camera.isPerspective() ? ogt_cam_mode_perspective : ogt_cam_mode_orthographic;
		cam.radius = (int)camera.farPlane();
		cam.fov = camera.fieldOfView();
		cam.frustum = camera.propertyf("frustum"); // TODO:
	}
	output_scene.num_cameras = output_cameras;
	if (output_cameras > 0) {
		output_scene.cameras = &cameras[0];
	}

	ogt_vox_palette &pal = output_scene.palette;
	ogt_vox_matl_array &mat = output_scene.materials;

	for (int i = 0; i < 256; ++i) {
		const core::RGBA &rgba = palette.colors[i];
		pal.color[i].r = rgba.r;
		pal.color[i].g = rgba.g;
		pal.color[i].b = rgba.b;
		pal.color[i].a = rgba.a;

		const core::RGBA &glowColor = palette.glowColors[i];
		if (glowColor.rgba != 0) {
			mat.matl[i].content_flags |= k_ogt_vox_matl_have_emit;
			mat.matl[i].type = ogt_matl_type::ogt_matl_type_emit;
			mat.matl[i].emit = 1.0f;
		}
	}

	uint32_t buffersize = 0;
	uint8_t *buffer = ogt_vox_write_scene(&output_scene, &buffersize);
	if (!buffer) {
		Log::error("Failed to write the scene");
		return false;
	}
	if (stream.write(buffer, buffersize) == -1) {
		Log::error("Failed to write to the stream");
		return false;
	}
	ogt_vox_free(buffer);

	for (ogt_vox_model &m : models) {
		core_free((void *)m.voxel_data);
	}

	return true;
}

} // namespace voxel
