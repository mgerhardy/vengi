/**
 * @file
 */

#pragma once

#include "app/tests/AbstractTest.h"
#include "core/ConfigVar.h"
#include "core/StringUtil.h"
#include "core/Var.h"
#include "core/Singleton.h"
#include "video/Renderer.h"
#include "video/ShaderManager.h"
#include "voxel/SurfaceExtractor.h"
#include <SDL3/SDL.h>

namespace video {

// can be used for parameterized tests
// inherit from ::testing::WithParamInterface<ShaderVarState>
// and use GetParam() to get the values
// don't forget to do INSTANTIATE_TEST_CASE_P
struct ShaderVarState {
	bool clientShadowMap = true;
	bool clientDebugShadow = false;
};

inline ::std::ostream& operator<<(::std::ostream& os, const ShaderVarState& state) {
	return os << "state["
			<< "clientShadowMap(" << state.clientShadowMap << "), "
			<< "clientDebugShadow(" << state.clientDebugShadow << ")"
			<< "]";
}

class AbstractGLTest : public app::AbstractTest {
protected:
	SDL_Window *_window = nullptr;
	RendererContext _ctx = nullptr;

	void setShaderVars(const ShaderVarState& val) {
		core::Var::get(cfg::RenderCheckerBoard, "false", core::CV_SHADER);
		core::Var::get(cfg::ClientShadowMap, "1", core::CV_SHADER)->setVal(val.clientShadowMap);
		core::Var::get(cfg::ClientDebugShadow, "1", core::CV_SHADER)->setVal(val.clientDebugShadow);
		core::Var::get(cfg::ClientShadowMapSize, "128", core::CV_SHADER);
		core::Var::get(cfg::ClientGamma, "1.0", core::CV_SHADER);
		core::Var::get(cfg::ClientBloom, "false");
		core::Var::get(cfg::RenderOutline, "false", core::CV_SHADER);
		core::Var::get(cfg::RenderNormals, "false", core::CV_SHADER);
		core::Var::get(cfg::ToneMapping, "0", core::CV_SHADER);
		core::Var::get(cfg::ClientDebugShadow, "false", core::CV_SHADER);
		core::Var::get(cfg::ClientDebugShadowMapCascade, "false", core::CV_SHADER);
		core::Var::get(cfg::VoxelMeshMode, core::string::toString((int)voxel::SurfaceExtractionType::Cubic), core::CV_SHADER);
		core::Singleton<ShaderManager>::getInstance().update();
	}

public:
	void SetUp() override {
		// 4.3 is the minimum version where compute shaders are supported
		core::Var::get(cfg::ClientOpenGLVersion, "4.3", core::CV_READONLY);
		core::Var::get(cfg::ClientMultiSampleBuffers, "0");
		core::Var::get(cfg::ClientMultiSampleSamples, "0");
		core::Var::get(cfg::ClientVSync, "false");
		core::Var::get(cfg::ClientDebugSeverity, "3");
		app::AbstractTest::SetUp();
#ifdef SDL_PLATFORM_WINDOWS
		GTEST_SKIP() << "Skipping because there are problems in the pipeline when running this headless";
#else
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			GTEST_SKIP() << "Failed to initialize SDL video subsystem";
			return;
		}
		video::setup();
		video::construct();
		_window = SDL_CreateWindow("test", 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
		if (_window != nullptr) {
			_ctx = video::createContext(_window);
			if (_ctx == nullptr) {
				GTEST_SKIP() << "Failed to create context";
			} else if (!video::init(640, 480, 1.0f)) {
				GTEST_SKIP() << "Failed to init video context";
			}
		} else {
			GTEST_SKIP() << "Failed to create window";
		}
#endif
	}

	void TearDown() override {
		app::AbstractTest::TearDown();
		if (_ctx != nullptr) {
			video::destroyContext(_ctx);
			_ctx = nullptr;
		}
		if (_window != nullptr) {
			SDL_DestroyWindow(_window);
			_window = nullptr;
		}
		SDL_Quit();
	}
};

class AbstractShaderTest :
		public AbstractGLTest,
		public ::testing::WithParamInterface<video::ShaderVarState> {
public:
	void SetUp() override {
		AbstractGLTest::SetUp();
		if (IsSkipped()) {
			return;
		}
		setShaderVars(GetParam());
	}
};

}

#define VIDEO_SHADERTEST(testname)								\
	INSTANTIATE_TEST_SUITE_P(									\
		ShaderVars,												\
		testname,												\
		::testing::Values(										\
			video::ShaderVarState{true, true},					\
			video::ShaderVarState{true, false},					\
			video::ShaderVarState{false, false},				\
			video::ShaderVarState{false, true}					\
		)														\
	);
