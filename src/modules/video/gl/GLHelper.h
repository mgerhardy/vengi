/**
 * @file
 */

#pragma once

#include "core/Log.h"
#include "core/collection/DynamicArray.h"
#include "flextGL.h"
#include "video/ShaderTypes.h"

#define GL_OFFSET_CAST(i) ((void*)(i))

namespace video {

namespace _priv {

int fillUniforms(Id program, ShaderUniforms& uniformMap, const core::String& shaderName, bool block);

#ifdef SDL_PLATFORM_WIN32
extern void __stdcall
#else
extern void
#endif
debugOutputCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
		const GLchar* message, const GLvoid* userParam);
extern GLenum checkFramebufferStatus(video::Id fbo);
extern void setupLimitsAndSpecs();
extern void setupFeatures();

}

}
