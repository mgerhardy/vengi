/**
 * @file
 */

#include "core/Log.h"
#include "core/collection/DynamicArray.h"
#include "video/RendererInterface.h"
#include "video/Shader.h"
#include "video/UniformBuffer.h"
#include "core/Assert.h"
#include "core/GLM.h"
#include "flextGL.h"
#include <SDL3/SDL.h>
#include <glm/gtc/type_ptr.hpp>

namespace video {

bool Shader::setAttributeLocation(const core::String& name, int location) {
	if (_program == InvalidId) {
		return false;
	}
	core_assert(glBindAttribLocation != nullptr);
	glBindAttribLocation(_program, location, name.c_str());
	checkError();
	return true;
}

int32_t Shader::getUniformBufferOffset(const char *name) {
	GLuint index;
	const GLchar *uniformNames[1];
	uniformNames[0] = name;
	core_assert(glGetUniformIndices != nullptr);
	glGetUniformIndices(_program, 1, uniformNames, &index);
	checkError();
	if (index == GL_INVALID_INDEX) {
		Log::error("Could not query uniform index for %s", name);
		return -1;
	}
	GLint offset;
	core_assert(glGetActiveUniformsiv != nullptr);
	glGetActiveUniformsiv(_program, 1, &index, GL_UNIFORM_OFFSET, &offset);
	checkError();
	GLint type;
	glGetActiveUniformsiv(_program, 1, &index, GL_UNIFORM_TYPE, &type);
	checkError();
	GLint size;
	glGetActiveUniformsiv(_program, 1, &index, GL_UNIFORM_SIZE, &size); // array length, not actual type size;
	checkError();
	GLint matrixStride;
	glGetActiveUniformsiv(_program, 1, &index, GL_UNIFORM_MATRIX_STRIDE, &matrixStride);
	checkError();
	GLint arrayStride;
	glGetActiveUniformsiv(_program, 1, &index, GL_UNIFORM_ARRAY_STRIDE, &arrayStride);
	checkError();
	Log::debug("%s: offset: %i, type: %i, size: %i, matrixStride: %i, arrayStride: %i", name, offset, type, size, matrixStride, arrayStride);
	return offset;
}

bool Shader::setUniformBuffer(const core::String& name, const UniformBuffer& buffer) {
	const Uniform* uniform = getUniform(name);
	if (uniform == nullptr) {
		Log::error("%s is no uniform", name.c_str());
		return false;
	}
	if (!uniform->block) {
		Log::error("%s is no uniform buffer", name.c_str());
		return false;
	}

	if (uniform->size != (int)buffer.size()) {
		Log::error("Uniform buffer %s: size %i differs from uploaded structure size %i", name.c_str(), uniform->size, (int)buffer.size());
		return false;
	}

	core_assert(glUniformBlockBinding != nullptr);
	glUniformBlockBinding(_program, (GLuint)uniform->blockIndex, (GLuint)uniform->blockBinding);
	checkError();
	addUsedUniform(uniform->location);
	return buffer.bind(uniform->blockIndex);
}

void Shader::setUniformi(int location, int value) const {
	if (checkUniformCache(location, &value, sizeof(value))) {
		core_assert(glUniform1i != nullptr);
		glUniform1i(location, value);
		checkError();
	}
	addUsedUniform(location);
}

void Shader::setVertexAttribute(int location, int size, DataType type, bool normalize, int stride, const void* buffer) const {
	core_assert_msg(getAttributeComponents(location) == -1 || getAttributeComponents(location) == size, "%i expected, but got %i components", getAttributeComponents(location), size);
#ifdef DEBUG
#if SDL_ASSERT_LEVEL > 0
	GLint vao = -1;
	core_assert(glGetIntegerv != nullptr);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
	core_assert_msg(vao > 0, "No vertex array object is bound");
#endif
#endif
	core_assert(glVertexAttribPointer != nullptr);
	glVertexAttribPointer(location, size, core::enumVal(type), normalize, stride, buffer);
	checkError();
}

void Shader::setVertexAttributeInt(int location, int size, DataType type, int stride, const void* buffer) const {
	core_assert_msg(getAttributeComponents(location) == -1 || getAttributeComponents(location) == size, "%i expected, but got %i components", getAttributeComponents(location), size);
#ifdef DEBUG
#if SDL_ASSERT_LEVEL > 0
	GLint vao = -1;
	core_assert(glGetIntegerv != nullptr);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
	core_assert_msg(vao > 0, "No vertex array object is bound");
#endif
#endif
	core_assert(glVertexAttribIPointer != nullptr);
	glVertexAttribIPointer(location, size, core::enumVal(type), stride, buffer);
	checkError();
}

bool Shader::enableVertexAttributeArray(int location) const {
	if (location < 0) {
		return false;
	}
#ifdef DEBUG
#if SDL_ASSERT_LEVEL > 0
	GLint vao = -1;
	core_assert(glGetIntegerv != nullptr);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
	core_assert_msg(vao > 0, "No vertex array object is bound");
#endif
#endif
	core_assert(glEnableVertexAttribArray != nullptr);
	glEnableVertexAttribArray(location);
	checkError();
	return true;
}

void Shader::disableVertexAttribute(int location) const {
#ifdef DEBUG
#if SDL_ASSERT_LEVEL > 0
	GLint vao = -1;
	core_assert(glGetIntegerv != nullptr);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
	core_assert_msg(vao > 0, "No vertex array object is bound");
#endif
#endif
	core_assert(location != -1);
	core_assert(glDisableVertexAttribArray != nullptr);
	glDisableVertexAttribArray(location);
	checkError();
}

bool Shader::setDivisor(int location, uint32_t divisor) const {
	if (location == -1) {
		return false;
	}
	core_assert(glVertexAttribDivisor != nullptr);
	glVertexAttribDivisor((GLuint)location, (GLuint)divisor);
	checkError();
	return true;
}

}
