/**
 * @file
 */

#include "MagicaVoxel.h"
#include "core/Color.h"
#include "core/GLMConst.h"
#include "core/Log.h"
#include "core/StandardLib.h"
#include "palette/Palette.h"
#include "scenegraph/CoordinateSystemUtil.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/SceneGraphNode.h"
#include "voxel/RawVolume.h"
#include <SDL_endian.h>
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/transform.hpp>
#define OGT_VOX_BIGENDIAN_SWAP32 SDL_SwapLE32
#define OGT_VOX_IMPLEMENTATION
#define ogt_assert(x, msg) core_assert_msg(x, "%s", msg)
#include "voxelformat/external/ogt_vox.h"

namespace voxelformat {

static glm::mat4 computeTransformationMatrix(const glm::mat4 transform, const glm::vec3 &pivot) {
	static const glm::mat4 shiftMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f));
	const glm::mat4 pivotMatrix = glm::translate(glm::mat4(1.0f), -pivot);
	const glm::mat4 combinedMatrix = transform * shiftMatrix * pivotMatrix;
	return combinedMatrix;
}

glm::mat4 ogtTransformToMat(const ogt_vox_instance &ogtInstance, uint32_t frameIdx, const ogt_vox_scene *scene,
							const ogt_vox_model *ogtModel) {
	ogt_vox_transform t = ogt_vox_sample_instance_transform_global(&ogtInstance, frameIdx, scene);
	const glm::vec4 col0(t.m00, t.m01, t.m02, t.m03);
	const glm::vec4 col1(t.m10, t.m11, t.m12, t.m13);
	const glm::vec4 col2(t.m20, t.m21, t.m22, t.m23);
	const glm::vec4 col3(t.m30, t.m31, t.m32, t.m33);
	const glm::vec3 &ogtPivot = ogtVolumePivot(ogtModel);
	return computeTransformationMatrix(glm::mat4{col0, col1, col2, col3}, ogtPivot);
}

void *_ogt_alloc(size_t size) {
	return core_malloc(size);
}

void _ogt_free(void *mem) {
	core_free(mem);
}

bool loadKeyFrames(scenegraph::SceneGraph &sceneGraph, scenegraph::SceneGraphNode &node,
				   const ogt_vox_instance &ogtInstance, const ogt_vox_scene *scene) {
	scenegraph::SceneGraphKeyFrames kf;

	const ogt_vox_anim_transform &transformAnim = ogtInstance.transform_anim;
	uint32_t numKeyframes = transformAnim.num_keyframes;
	Log::debug("Load %d keyframes", numKeyframes);
	kf.reserve(numKeyframes);
	const ogt_vox_model *ogtModel = scene->models[ogtInstance.model_index];
	for (uint32_t keyFrameIdx = 0; keyFrameIdx < numKeyframes; ++keyFrameIdx) {
		const ogt_vox_keyframe_transform &keyFrameTransform = transformAnim.keyframes[keyFrameIdx];
		const uint32_t frameIdx = keyFrameTransform.frame_index;
		const glm::mat4 ogtMat = ogtTransformToMat(ogtInstance, frameIdx, scene, ogtModel);
		scenegraph::SceneGraphKeyFrame sceneGraphKeyFrame;
		sceneGraphKeyFrame.frameIdx = (scenegraph::FrameIndex)frameIdx;
		sceneGraphKeyFrame.interpolation = scenegraph::InterpolationType::Linear;
		sceneGraphKeyFrame.longRotation = false;
		scenegraph::SceneGraphTransform &transform = sceneGraphKeyFrame.transform();
		transform.setLocalMatrix(scenegraph::convertCoordinateSystem(scenegraph::CoordinateSystem::MagicaVoxel, ogtMat));
		kf.push_back(sceneGraphKeyFrame);
	}
	return node.setKeyFrames(kf);
}

void loadPaletteFromScene(const ogt_vox_scene *scene, palette::Palette &palette) {
	palette.setSize(0);
	int palIdx = 0;
	for (int i = 0; i < lengthof(scene->palette.color) - 1; ++i) {
		const ogt_vox_rgba color = scene->palette.color[(i + 1) & 255];
		palette.setColor(palIdx, core::RGBA(color.r, color.g, color.b, color.a));
		const ogt_vox_matl &matl = scene->materials.matl[(i + 1) & 255];
		if (matl.content_flags & k_ogt_vox_matl_have_metal) {
			palette.setMetal(palIdx, matl.metal);
		}
		if (matl.content_flags & k_ogt_vox_matl_have_rough) {
			palette.setRoughness(palIdx, matl.rough);
		}
		if (matl.content_flags & k_ogt_vox_matl_have_spec) {
			palette.setSpecular(palIdx, matl.spec);
		}
		if (matl.content_flags & k_ogt_vox_matl_have_ior) {
			palette.setIndexOfRefraction(palIdx, matl.ior);
		}
		if (matl.content_flags & k_ogt_vox_matl_have_att) {
			palette.setAttenuation(palIdx, matl.att);
		}
		if (matl.content_flags & k_ogt_vox_matl_have_flux) {
			palette.setFlux(palIdx, matl.flux);
		}
		if (matl.content_flags & k_ogt_vox_matl_have_emit) {
			palette.setEmit(palIdx, matl.emit);
		}
		if (matl.content_flags & k_ogt_vox_matl_have_ldr) {
			palette.setLowDynamicRange(palIdx, matl.ldr);
		}
		// TODO: not sure how to handle this yet
		if (matl.content_flags & k_ogt_vox_matl_have_trans) {
			palette.setAlpha(palIdx, matl.trans);
		} else if (matl.content_flags & k_ogt_vox_matl_have_alpha) {
			palette.setAlpha(palIdx, matl.alpha);
		}
		if (matl.content_flags & k_ogt_vox_matl_have_d) {
			palette.setGlossiness(palIdx, matl.d);
		}
		if (matl.content_flags & k_ogt_vox_matl_have_sp) {
			palette.setSp(palIdx, matl.sp);
		}
		if (matl.content_flags & k_ogt_vox_matl_have_g) {
			palette.setGlossiness(palIdx, matl.g);
		}
		if (matl.content_flags & k_ogt_vox_matl_have_media) {
			palette.setMedia(palIdx, matl.media);
		}
		++palIdx;
	}
	int n = 0;
	for (int i = 0; i < palette::PaletteMaxColors; ++i) {
		if (palette.color(i).a > 0) {
			n = i + 1;
		}
	}
	if (n > 0) {
		palette.setSize(n);
	}
	Log::debug("vox load color count: %i", palette.colorCount());
}

bool loadPaletteFromBuffer(const uint8_t *buffer, size_t size, palette::Palette &palette) {
	const ogt_vox_scene *scene = ogt_vox_read_scene_with_flags(buffer, size, 0);
	if (scene == nullptr) {
		Log::error("Could not load scene");
		return false;
	}
	loadPaletteFromScene(scene, palette);
	ogt_vox_destroy_scene(scene);
	return true;
}

void printDetails(const ogt_vox_scene *scene) {
	Log::debug("vox groups: %u", scene->num_groups);
	for (uint32_t i = 0; i < scene->num_groups; ++i) {
		if (scene->groups[i].name) {
			Log::debug(" %u: %s", i, scene->groups[i].name);
		}
	}
	Log::debug("vox instances: %u", scene->num_instances);
	for (uint32_t i = 0; i < scene->num_instances; ++i) {
		if (scene->instances[i].name) {
			Log::debug(" %u: %s", i, scene->instances[i].name);
		}
	}
	Log::debug("vox layers: %u", scene->num_layers);
	for (uint32_t i = 0; i < scene->num_layers; ++i) {
		if (scene->layers[i].name) {
			Log::debug(" %u: %s", i, scene->layers[i].name);
		}
	}
	Log::debug("vox models: %u", scene->num_models);
	Log::debug("vox cameras: %u", scene->num_cameras);
}

#ifdef DEBUG
static bool checkRotationRow(const glm::vec3 &vec) {
	for (int i = 0; i < 3; i++) {
		if (vec[i] == 1.0f || vec[i] == -1.0f) {
			return true;
		}
		core_assert_msg(vec[i] == 0.0f, "rotation vector should contain only 0.0f, 1.0f, or -1.0f");
	}
	return false;
}
#endif

void checkRotation(const ogt_vox_transform &transform) {
#ifdef DEBUG
	core_assert(checkRotationRow({transform.m00, transform.m01, transform.m02}));
	core_assert(checkRotationRow({transform.m10, transform.m11, transform.m12}));
	core_assert(checkRotationRow({transform.m20, transform.m21, transform.m22}));
#endif
}

void loadCameras(const ogt_vox_scene *scene, scenegraph::SceneGraph &sceneGraph) {
	for (uint32_t n = 0; n < scene->num_cameras; ++n) {
		const ogt_vox_cam &c = scene->cameras[n];
		const glm::vec3 target(c.focus[0], c.focus[2], c.focus[1]);
		const glm::vec3 angles(c.angle[0], -c.angle[1], c.angle[2]);
		const glm::vec3 rangles = glm::radians(angles);
		const glm::quat quat(rangles);
		const float distance = (float)c.radius;
		// const glm::vec3 forward(glm::cos(rangles.x) * glm::sin(rangles.y), glm::sin(rangles.x),
		// 					  glm::cos(rangles.x) * glm::cos(rangles.y));
		const glm::vec3 &forward = glm::conjugate(quat) * glm::forward();
		const glm::vec3 &backward = -forward;
		const glm::vec3 &newPosition = target + backward * distance;
		// const glm::vec3 newPosition =
		// 	glm::vec3(target.x + (float)size.x / 2.0f, target.y, target.z + (float)size.z / 2.0f) -
		// 	distance * forward;
		const glm::mat4 &orientation = glm::mat4_cast(quat);
		const glm::mat4 &viewMatrix = glm::translate(orientation, -newPosition);

		{
			scenegraph::SceneGraphNodeCamera camNode;
			camNode.setName(core::String::format("Camera %u", c.camera_id));
			scenegraph::SceneGraphTransform transform;
			transform.setWorldMatrix(viewMatrix);
			const scenegraph::KeyFrameIndex keyFrameIdx = 0;
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
}

MVModelToNode::MVModelToNode() : volume(nullptr), nodeId(InvalidNodeId) {
}

MVModelToNode::~MVModelToNode() {
	delete volume;
}

const char *instanceName(const ogt_vox_scene *scene, const ogt_vox_instance &instance) {
	const ogt_vox_layer &layer = scene->layers[instance.layer_index];
	const char *name = instance.name == nullptr ? layer.name : instance.name;
	if (name == nullptr) {
		name = "";
	}
	return name;
}

core::RGBA instanceColor(const ogt_vox_scene *scene, const ogt_vox_instance &instance) {
	const ogt_vox_layer &layer = scene->layers[instance.layer_index];
	const core::RGBA col(layer.color.r, layer.color.g, layer.color.b, layer.color.a);
	return col;
}

bool instanceHidden(const ogt_vox_scene *scene, const ogt_vox_instance &instance) {
	// check if this instance is hidden in the .vox file
	if (instance.hidden) {
		return true;
	}
	// check if this instance is part of a hidden layer in the .vox file
	if (scene->layers[instance.layer_index].hidden) {
		return true;
	}
	// check if this instance is part of a hidden group
	if (instance.group_index != k_invalid_group_index && instance.group_index < scene->num_groups &&
		scene->groups[instance.group_index].hidden) {
		return true;
	}
	return false;
}

core::DynamicArray<MVModelToNode> loadModels(const ogt_vox_scene *scene, const palette::Palette &palette) {
	core::DynamicArray<MVModelToNode> models;
	models.reserve(scene->num_models);
	for (uint32_t i = 0; i < scene->num_models; ++i) {
		const ogt_vox_model *ogtModel = scene->models[i];
		if (ogtModel == nullptr) {
			models.emplace_back(nullptr, InvalidNodeId);
			continue;
		}
		voxel::Region region(glm::ivec3(0),
							 glm::ivec3(ogtModel->size_x - 1, ogtModel->size_z - 1, ogtModel->size_y - 1));
		voxel::RawVolume *v = new voxel::RawVolume(region);

		const uint8_t *ogtVoxel = ogtModel->voxel_data;
		for (uint32_t z = 0; z < ogtModel->size_z; ++z) {
			for (uint32_t y = 0; y < ogtModel->size_y; ++y) {
				for (uint32_t x = 0; x < ogtModel->size_x; ++x, ++ogtVoxel) {
					if (ogtVoxel[0] == 0) {
						continue;
					}
					const voxel::Voxel voxel = voxel::createVoxel(palette, ogtVoxel[0] - 1);
					v->setVoxel(region.getUpperX() - (int)x, (int)z, (int)y, voxel);
				}
			}
		}
		models.emplace_back(v, InvalidNodeId);
	}
	return models;
}

} // namespace voxelformat
