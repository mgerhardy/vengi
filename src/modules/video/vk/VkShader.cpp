/**
 * @file
 */

#include "video/Shader.h"
#include "video/UniformBuffer.h"
#include "core/Assert.h"
#include "core/GLM.h"
#include "flextVk.h"
#include <SDL3/SDL.h>
#include <glm/gtc/type_ptr.hpp>

namespace video {

bool Shader::setAttributeLocation(const core::String& name, int location) {
	if (_program == InvalidId) {
		return false;
	}
	// TODO
	checkError();
	return true;
}

int32_t Shader::getUniformBufferOffset(const char *name) {
	return false;
}

bool Shader::setUniformBuffer(const core::String& name, const UniformBuffer& buffer) {
	const Uniform* uniform = getUniform(name);
	if (uniform == nullptr) {
		return false;
	}
	if (!uniform->block) {
		return false;
	}

	// TODO
	checkError();
	addUsedUniform(uniform->location);
	return false;
}

void Shader::setUniformi(int location, int value) const {
	if (checkUniformCache(location, &value, sizeof(value))) {
		// TODO
		checkError();
	}
	addUsedUniform(location);
}

void Shader::setVertexAttribute(int location, int size, DataType type, bool normalize, int stride, const void* buffer) const {
	core_assert_msg(getAttributeComponents(location) == -1 || getAttributeComponents(location) == size, "%i expected, but got %i components", getAttributeComponents(location), size);
	// TODO
	checkError();
}

void Shader::setVertexAttributeInt(int location, int size, DataType type, int stride, const void* buffer) const {
	core_assert_msg(getAttributeComponents(location) == -1 || getAttributeComponents(location) == size, "%i expected, but got %i components", getAttributeComponents(location), size);
	// TODO
	checkError();
}

bool Shader::enableVertexAttributeArray(int location) const {
	if (location < 0) {
		return false;
	}
	// TODO
	checkError();
	return true;
}

void Shader::disableVertexAttribute(int location) const {
	core_assert(location != -1);
	// TODO
	checkError();
}

bool Shader::setDivisor(int location, uint32_t divisor) const {
	if (location == -1) {
		return false;
	}
	// TODO
	checkError();
	return true;
}

}
