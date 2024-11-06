/**
 * @file
 */

#include "GLHelper.h"
#include "core/ArrayLength.h"
#include "video/Renderer.h"
#include "core/Common.h"
#include "core/collection/List.h"
#include "core/Log.h"
#include <SDL3/SDL.h>

namespace video {

namespace _priv {

static int _recompileErrors = 0;

#ifdef SDL_PLATFORM_WIN32
void __stdcall
#else
void
#endif
debugOutputCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if (id == 131218) {
		++_recompileErrors;
		if (_recompileErrors <= 10) {
			return;
		}
		_recompileErrors = 0;
	} else if (id == 131185) {
		// ignore hints that GL_STATIC_DRAW is used...
		return;
	}
	void (*log)(const char* msg, ...);
	const char* sourceStr;
	switch (source) {
	case GL_DEBUG_SOURCE_API_ARB:
		sourceStr = "api";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
		sourceStr = "window";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
		sourceStr = "third party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION_ARB:
		sourceStr = "app";
		break;
	case GL_DEBUG_SOURCE_OTHER_ARB:
		sourceStr = "other";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
		sourceStr = "shader";
		break;
	default:
		sourceStr = "unknown";
		break;
	}
	const char* typeStr;
	switch (type) {
	case GL_DEBUG_TYPE_ERROR_ARB:
		typeStr = "ERROR";
		log = Log::error;
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
		typeStr = "DEPRECATED_BEHAVIOR";
		log = Log::warn;
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
		typeStr = "UNDEFINED_BEHAVIOR";
		log = Log::error;
		break;
	case GL_DEBUG_TYPE_PORTABILITY_ARB:
		typeStr = "PORTABILITY";
		log = Log::warn;
		break;
	case GL_DEBUG_TYPE_PERFORMANCE_ARB:
		typeStr = "PERFORMANCE";
		log = Log::warn;
		break;
	case GL_DEBUG_TYPE_OTHER_ARB:
		typeStr = "OTHER";
		log = Log::info;
		break;
	default:
		typeStr = "<unknown>";
		log = Log::debug;
		break;
	}
	const char* sevStr;
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW_ARB:
		sevStr = "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM_ARB:
		sevStr = "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH_ARB:
		sevStr = "HIGH";
		log = Log::error;
		break;
	default:
		sevStr = "<unknown>";
		break;
	}
	//core_assert_msg(type == GL_DEBUG_TYPE_OTHER_ARB, "GL msg type: %s, src: %s, id: %d, severity: %s\nmsg: %s", typeStr, sourceStr, id, sevStr, message);
	log("GL msg type: %s, src: %s, id: %d, severity: %s\nmsg: %s", typeStr, sourceStr, id, sevStr, message);
}

GLenum checkFramebufferStatus(video::Id fbo) {
	GLenum status;
	if (useFeature(Feature::DirectStateAccess)) {
		core_assert(glCheckNamedFramebufferStatus != nullptr);
		status = glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER);
	} else {
		core_assert(glCheckFramebufferStatus != nullptr);
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	}
	if (status == GL_FRAMEBUFFER_COMPLETE) {
		return status;
	}
	switch (status) {
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		Log::error("FB error, incomplete attachment");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		Log::error("FB error, incomplete missing attachment");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		Log::error("FB error, incomplete draw buffer");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		Log::error("FB error, incomplete read buffer");
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		Log::error("FB error, framebuffer unsupported");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		Log::error("FB error, incomplete multisample");
		break;
	default:
		Log::error("FB error, status: %i", (int)status);
		break;
	}
	return status;
}

void setupLimitsAndSpecs() {
	core_assert(glGetIntegerv != nullptr);
	glGetIntegerv(GL_MAX_SAMPLES, &renderState().limits[core::enumVal(Limit::MaxSamples)]);
	checkError();
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &renderState().limits[core::enumVal(Limit::MaxTextureSize)]);
	checkError();
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &renderState().limits[core::enumVal(Limit::MaxCubeMapTextureSize)]);
	checkError();
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &renderState().limits[core::enumVal(Limit::MaxViewPortWidth)]);
	checkError();
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &renderState().limits[core::enumVal(Limit::MaxDrawBuffers)]);
	checkError();
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &renderState().limits[core::enumVal(Limit::MaxVertexAttribs)]);
	checkError();
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &renderState().limits[core::enumVal(Limit::MaxCombinedTextureImageUnits)]);
	checkError();
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &renderState().limits[core::enumVal(Limit::MaxVertexTextureImageUnits)]);
	checkError();
	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &renderState().limits[core::enumVal(Limit::MaxElementIndices)]);
	checkError();
	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &renderState().limits[core::enumVal(Limit::MaxElementVertices)]);
	checkError();
	glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &renderState().limits[core::enumVal(Limit::MaxFragmentInputComponents)]);
	checkError();
	if (hasFeature(Feature::ComputeShaders)) {
		core_assert(glGetIntegeri_v != nullptr);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &renderState().limits[core::enumVal(Limit::MaxComputeWorkGroupCountX)]);
		checkError();
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &renderState().limits[core::enumVal(Limit::MaxComputeWorkGroupCountY)]);
		checkError();
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &renderState().limits[core::enumVal(Limit::MaxComputeWorkGroupCountZ)]);
		checkError();
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &renderState().limits[core::enumVal(Limit::MaxComputeWorkGroupSizeX)]);
		checkError();
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &renderState().limits[core::enumVal(Limit::MaxComputeWorkGroupSizeY)]);
		checkError();
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &renderState().limits[core::enumVal(Limit::MaxComputeWorkGroupSizeZ)]);
		checkError();
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &renderState().limits[core::enumVal(Limit::MaxComputeWorkGroupInvocations)]);
		checkError();
	}
#ifdef GL_MAX_VERTEX_UNIFORM_VECTORS
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &renderState().limits[core::enumVal(Limit::MaxVertexUniformComponents)]);
	checkError();
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &renderState().limits[core::enumVal(Limit::MaxFragmentUniformComponents)]);
	checkError();
#else
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &renderState().limits[core::enumVal(Limit::MaxVertexUniformComponents)]);
	checkError();
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &renderState().limits[core::enumVal(Limit::MaxFragmentUniformComponents)]);
	checkError();
#endif
	GLint uniformBufferAlignment;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBufferAlignment);
	renderState().specs[core::enumVal(Spec::UniformBufferAlignment)] = uniformBufferAlignment;
	checkError();
	GLint maxUniformBufferSize;
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
	renderState().limits[core::enumVal(Limit::MaxUniformBufferSize)] = maxUniformBufferSize;
	checkError();
	GLint maxUniformBufferBindings;
	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUniformBufferBindings);
	renderState().limits[core::enumVal(Limit::MaxUniformBufferBindings)] = maxUniformBufferBindings;
	checkError();

	if (hasFeature(Feature::ShaderStorageBufferObject)) {
		GLint shaderStorageBufferOffsetAlignment;
		glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &shaderStorageBufferOffsetAlignment);
		renderState().specs[core::enumVal(Spec::ShaderStorageBufferOffsetAlignment)] = shaderStorageBufferOffsetAlignment;
		checkError();
		GLint maxShaderStorageBlockSize;
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxShaderStorageBlockSize);
		renderState().limits[core::enumVal(Limit::MaxShaderStorageBufferSize)] = maxShaderStorageBlockSize;
		checkError();
	}

	Log::debug("GL_MAX_ELEMENTS_VERTICES: %i", renderState().limits[core::enumVal(Limit::MaxElementVertices)]);
	Log::debug("GL_MAX_ELEMENTS_INDICES: %i", renderState().limits[core::enumVal(Limit::MaxElementIndices)]);
	Log::debug("GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT: %i", (int)renderState().specs[core::enumVal(Spec::UniformBufferAlignment)]);
	Log::debug("GL_MAX_UNIFORM_BLOCK_SIZE: %i", (int)renderState().limits[core::enumVal(Limit::MaxUniformBufferSize)]);
	Log::debug("GL_MAX_UNIFORM_BUFFER_BINDINGS: %i", (int)renderState().limits[core::enumVal(Limit::MaxUniformBufferBindings)]);
}

void setupFeatures() {
	const core::List<const char *> extensionArray[] = {
		{"GL_ARB_texture_compression_s3tc", "GL_ARB_compressed_texture_s3tc", "GL_ARB_texture_compression_dxt1"},
		{"GL_ARB_texture_compression_pvrtc", "GL_ARB_compressed_texture_pvrtc"},
		{},
		{"GL_ARB_compressed_ATC_texture", "GL_ARB_compressed_texture_atc"},
		{"GL_ARB_texture_float"},
		{"GL_ARB_texture_half_float"},
		{"GL_ARB_instanced_arrays"},
		{"GL_ARB_debug_output"},
		// the primary difference between ARB and EXT is that ARB requires the use of
		// glCreateResource rather than working from glGenResource object handles.
		// https://www.opengl.org/registry/specs/ARB/direct_state_access.txt
		{"GL_ARB_direct_state_access"},
		{"GL_ARB_buffer_storage"},
		{"GL_ARB_multi_draw_indirect"},
		{"GL_ARB_compute_shader"},
		{"GL_ARB_transform_feedback2"},
		{"GL_ARB_shader_storage_buffer_object"}
	};
	static_assert(core::enumVal(Feature::Max) == (int)lengthof(extensionArray), "Array sizes don't match for Feature enum");

	int numExts;
	core_assert(glGetIntegerv != nullptr);
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
	Log::debug("OpenGL extensions:");
	for (int i = 0; i < numExts; ++i) {
		const char *extensionStr = (const char *) glGetStringi(GL_EXTENSIONS, i);
		Log::debug("ext: %s", extensionStr);
	}

	for (size_t i = 0; i < lengthof(extensionArray); ++i) {
		const core::List<const char *>& extStrVector = extensionArray[i];
		for (const char *extStr : extStrVector) {
			renderState().features[i] = SDL_GL_ExtensionSupported(extStr);
			if (renderState().features[i]) {
				Log::debug("Detected feature: %s", extStr);
				break;
			}
		}
	}

	int mask = 0;
	if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &mask) != -1) {
		if ((mask & SDL_GL_CONTEXT_PROFILE_CORE) != 0) {
			renderState().features[core::enumVal(Feature::TextureCompressionDXT)] = true;
			renderState().features[core::enumVal(Feature::InstancedArrays)] = true;
			renderState().features[core::enumVal(Feature::TextureFloat)] = true;
		}
	}

#ifdef GL_CLIP_ORIGIN
	GLenum clipOrigin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&clipOrigin); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
	if (clipOrigin == GL_UPPER_LEFT) {
		s.clipOriginLowerLeft = false;
	}
#endif

#ifdef USE_OPENGLES
	renderState().features[core::enumVal(Feature::TextureFloat)] = true;
	renderState().features[core::enumVal(Feature::TextureHalfFloat)] = true;
	renderState().features[core::enumVal(Feature::InstancedArrays)] = true;
	renderState().features[core::enumVal(Feature::TextureCompressionETC2)] = true;
#else
	renderState().features[core::enumVal(Feature::TextureHalfFloat)] = renderState().features[core::enumVal(Feature::TextureFloat)];
#endif
}

int fillUniforms(Id program, ShaderUniforms& uniformMap, const core::String& shaderName, bool block) {
	GLenum activeEnum;
	GLenum activeMaxLengthEnum;
	if (block) {
		activeEnum = GL_ACTIVE_UNIFORM_BLOCKS;
		activeMaxLengthEnum = GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH;
	} else {
		activeEnum = GL_ACTIVE_UNIFORMS;
		activeMaxLengthEnum = GL_ACTIVE_UNIFORM_MAX_LENGTH;
	}
	GLint numUniforms = 0;
	glGetProgramiv(program, activeEnum, &numUniforms);
	GLint uniformNameSize = 0;
	glGetProgramiv(program, activeMaxLengthEnum, &uniformNameSize);
	char name[4096];
	if (uniformNameSize + 1 >= (int)sizeof(name)) {
		return 0;
	}

	const char *shaderNameC = shaderName.c_str();
	for (int i = 0; i < numUniforms; i++) {
		int location;
		if (block) {
			core_assert(glGetActiveUniformBlockName != nullptr);
			glGetActiveUniformBlockName(program, i, uniformNameSize, nullptr, name);
			core_assert(glGetUniformBlockIndex != nullptr);
			location = glGetUniformBlockIndex(program, name);
			if (location < 0) {
				Log::debug("Could not get uniform block location for %s is %i (shader %s)", name, location, shaderNameC);
				continue;
			}
			Log::debug("Got uniform location for %s is %i (shader %s)", name, location, shaderNameC);
		} else {
			GLint size = 0;
			GLenum type = 0;
			core_assert(glGetActiveUniform != nullptr);
			glGetActiveUniform(program, i, uniformNameSize, nullptr, &size, &type, name);
			core_assert(glGetUniformLocation != nullptr);
			location = glGetUniformLocation(program, name);
			if (location < 0) {
				Log::debug("Could not get uniform location for %s is %i (shader %s)", name, location, shaderNameC);
				continue;
			}
			Log::debug("Got uniform location for %s is %i (shader %s)", name, location, shaderNameC);
		}
		char* array = SDL_strchr(name, '[');
		if (array != nullptr) {
			*array = '\0';
		}
		Uniform uniform;
		uniform.location = location;
		uniform.block = block;
		if (block) {
			core_assert(glGetUniformBlockIndex != nullptr);
			uniform.blockIndex = glGetUniformBlockIndex(program, name);
			core_assert(glGetActiveUniformBlockiv != nullptr);
			glGetActiveUniformBlockiv(program, location, GL_UNIFORM_BLOCK_DATA_SIZE, &uniform.size);
			uniform.blockBinding = i;
		}
		uniformMap.put(core::String(name), uniform);
	}
	return numUniforms;
}

}

}
