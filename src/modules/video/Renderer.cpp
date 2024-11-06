/**
 * @file
 */

#include "Renderer.h"
#include "core/ArrayLength.h"
#include "core/Enum.h"
#include "core/Log.h"
#include "core/Assert.h"
#include "TextureConfig.h"
#include "core/Var.h"
#include <SDL3/SDL_video.h>

namespace video {

static const char *featuresArray[] = {
	"r_texturecompressiondxt",	"r_texturecompressionpvrtc",
	"r_texturecompressionetc2", "r_texturecompressionatc",
	"r_texturefloat",			"r_texturehalffloat",
	"r_instancedarrays",		"r_debugoutput",
	"r_directstateaccess",		"r_bufferstorage",
	"r_multidrawindirect",		"r_computeshaders",
	"r_transformfeedback",		"r_shaderstoragebufferobject"
};
static_assert(core::enumVal(Feature::Max) == (int)lengthof(featuresArray), "Array sizes don't match with Feature enum");
static core::VarPtr featureVars[core::enumVal(Feature::Max)];

static RenderState s;

void construct() {
	for (int i = 0; i < core::enumVal(Feature::Max); ++i) {
		featureVars[i] = core::Var::get(featuresArray[i], "false", "Renderer feature cvar", core::Var::boolValidator);
	}
}

DataType mapIndexTypeBySize(size_t size) {
	if (size == 4u) {
		return DataType::UnsignedInt;
	}
	if (size == 2u) {
		return DataType::UnsignedShort;
	}
	core_assert(size == 1u);
	return DataType::UnsignedByte;
}

void deleteRenderbuffer(Id& id) {
	if (id == InvalidId) {
		return;
	}
	deleteRenderbuffers(1, &id);
	id = InvalidId;
}

void deleteFramebuffer(Id& id) {
	if (id == InvalidId) {
		return;
	}
	deleteFramebuffers(1, &id);
	id = InvalidId;
}

void deleteTexture(Id& id) {
	if (id == InvalidId) {
		return;
	}
	deleteTextures(1, &id);
	id = InvalidId;
}

Id genTexture(const TextureConfig &cfg) {
	Id id;
	genTextures(cfg, 1, &id);
	return id;
}

void deleteBuffer(Id& id) {
	if (id == InvalidId) {
		return;
	}
	deleteBuffers(1, &id);
	id = InvalidId;
}

Id genVertexArray() {
	Id id;
	genVertexArrays(1, &id);
	return id;
}

Id genBuffer() {
	Id id;
	genBuffers(1, &id);
	return id;
}

Id genRenderbuffer() {
	Id id;
	genRenderbuffers(1, &id);
	return id;
}

Id genFramebuffer() {
	Id id;
	genFramebuffers(1, &id);
	return id;
}

void disableDebug() {
	if (!hasFeature(Feature::DebugOutput)) {
		return;
	}
	disable(State::DebugOutput);
	Log::info("disable render debug messages");
}

bool checkLimit(int amount, Limit l) {
	const int v = renderState().limit(l);
	if (v <= 0) {
		Log::trace("No limit found for %i", (int)l);
		return true;
	}
	return v >= amount;
}

bool useFeature(Feature feature) {
	if (!hasFeature(feature)) {
		return false;
	}
	const core::VarPtr& v = featureVars[core::enumVal(feature)];
	if (!v) {
		return true;
	}
	return v->boolVal();
}

RenderState& renderState() {
	return s;
}

}
