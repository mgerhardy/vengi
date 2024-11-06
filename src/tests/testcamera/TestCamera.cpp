/**
 * @file
 */

#include "TestCamera.h"
#include "testcore/TestAppMain.h"
#include "core/Log.h"
#include "util/CustomButtonNames.h"
#include <SDL3/SDL.h>

TestCamera::TestCamera(const io::FilesystemPtr& filesystem, const core::TimeProviderPtr& timeProvider) :
		Super(filesystem, timeProvider) {
	init(ORGANISATION, "testcamera");
	setCameraMotion(false);
	//setRenderPlane(true);
	setRenderAxis(true);
}

app::AppState TestCamera::onInit() {
	app::AppState state = Super::onInit();
	if (state != app::AppState::Running) {
		return state;
	}

	const float nearPlane = 5.0f;

	const int ents = (int)_entities.size();
	const int rows = ents / 5;
	const int cols = ents / rows;
	const float distance = 40.0f;
	const float deltaY = (float)rows / 2.0f * -distance;
	const float deltaX = (float)cols / 2.0f * -distance;
	core_assert(ents - rows * cols == 0);
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			const glm::vec3 p(deltaX + distance * (float)col, deltaY + distance * (float)row, 50.0f);
			_entities[row * cols + col].setPosition(p).init();
		}
	}

	const glm::vec4 colors[CAMERAS] = { core::Color::Red(), core::Color::Yellow(), core::Color::Pink() };
	static_assert(CAMERAS == 3, "Unexpected amount of cameras");
	for (int i = 0; i < CAMERAS; ++i) {
		bool renderAABB = i == 0;
		bool renderSplitFrustum = i == 1;
		bool targetCamera = i == 0;
		bool ortho = i == 2;

		_renderCamera[i].setSize(ortho ? glm::ivec2(100, 50) : ortho ? glm::ivec2(100, 50) : windowDimension());
		_renderCamera[i].setOmega(glm::vec3(0.0f, 0.1f, 0.0f));

		_renderCamera[i].setWorldPosition(glm::zero<glm::vec3>());
		_renderCamera[i].lookAt(glm::vec3(10.0f, 70.0f, 10.0f));
		_renderCamera[i].setNearPlane(nearPlane);
		_renderCamera[i].setFarPlane(40.0f);

		if (targetCamera) {
			_renderCamera[i].setRotationType(video::CameraRotationType::Target);
		}
		if (ortho) {
			_renderCamera[i].setMode(video::CameraMode::Orthogonal);
		}
		_renderCamera[i].update(0l);

		if (!_frustums[i].init(colors[i], renderSplitFrustum ? 4 : 0)) {
			return app::AppState::InitFailure;
		}
		_frustums[i].setRenderAABB(renderAABB);
	}

	resetCameraPosition();

	return state;
}

void TestCamera::resetCameraPosition() {
	camera().setWorldPosition(glm::vec3(0.0f, 100.0f, 250.0f));
	camera().setAngles(0.0f, 0.0f, 0.0f);
	camera().lookAt(glm::vec3(0.0f));
}

void TestCamera::doRender() {
	video::Camera& c = _renderCamera[_targetCamera];
	c.update(_deltaFrameSeconds);
	_frustums[_targetCamera].render(camera(), c);
	for (FrustumEntity& e : _entities) {
		e.cull(c);
		e.render(camera());
	}
}

void TestCamera::onRenderUI() {
	const char *cameraRotType;
	const video::Camera& targetCamera = _renderCamera[_targetCamera];
	const video::CameraRotationType rotType = targetCamera.rotationType();
	switch (rotType) {
	case video::CameraRotationType::Target:
		cameraRotType = "R: Target";
		break;
	case video::CameraRotationType::Eye:
	default:
		cameraRotType = "R: Eye";
		break;
	}
	const char *mode = "";
	if (_targetCamera == 1) {
		mode = "Frustum split";
	}
	ImGui::Text("Space: toggle camera");
	ImGui::Text("Shift/MouseMove: rotate");
	ImGui::Text("Backspace: toggle aabb");
	ImGui::Text("0: reset position");
	ImGui::Text("Shift/+ Shift/-: far plane");
	ImGui::Text("Ctrl/Shift/+ Ctrl/Shift/-: near plane");
	ImGui::Text("Shift/MouseWheel: far plane");
	ImGui::Text("Ctrl/Shift/MouseWheel: near plane");
	ImGui::Text("Sphere: red = outside, green = inside, purple = touching");
	ImGui::Text("Camera: %s (%i) %s", cameraRotType, _targetCamera + 1, mode);
	if (_frustums[_targetCamera].renderAABB()) {
		const math::AABB<float>&& aabb = targetCamera.aabb();
		ImGui::Text("AABB(mins(%.2f:%.2f:%.2f), maxs(%.2f:%.2f:%.2f))", aabb.getLowerX(), aabb.getLowerY(), aabb.getLowerZ(), aabb.getUpperX(), aabb.getUpperY(), aabb.getUpperZ());
	}
	Super::onRenderUI();
}

app::AppState TestCamera::onRunning() {
	app::AppState state = Super::onRunning();
	video::Camera& c = _renderCamera[_targetCamera];
	const SDL_Keymod mods = SDL_GetModState();
	if (mods & SDL_KMOD_SHIFT) {
		c.rotate(glm::vec3(_mouseRelativePos.y, _mouseRelativePos.x, 0.0f) * _rotationSpeed->floatVal());
	}
	camera().setTarget(c.worldPosition());
	return state;
}

app::AppState TestCamera::onCleanup() {
	for (int i = 0; i < CAMERAS; ++i) {
		_frustums[i].shutdown();
	}

	const int ents = (int)_entities.size();
	for (int i = 0; i < ents; ++i) {
		_entities[i].shutdown();
	}

	return Super::onCleanup();
}

bool TestCamera::onMouseWheel(void *windowHandle, float x, float y, int32_t mouseId) {
	const SDL_Keymod mods = SDL_GetModState();
	if (mods & SDL_KMOD_SHIFT) {
		video::Camera& c = _renderCamera[_targetCamera];
		if (mods & SDL_KMOD_CONTROL) {
			c.setNearPlane(c.nearPlane() + (float)y);
		} else {
			c.setFarPlane(c.farPlane() + (float)y);
		}
		return true;
	}

	return Super::onMouseWheel(windowHandle, x, y, mouseId);
}

bool TestCamera::onKeyPress(void *windowHandle, int32_t key, int16_t modifier) {
	const bool retVal = Super::onKeyPress(windowHandle, key, modifier);
	if (key == SDLK_SPACE) {
		_targetCamera++;
		_targetCamera %= CAMERAS;
	}

	if (key == SDLK_BACKSPACE) {
		const bool aabb = _frustums[_targetCamera].renderAABB();
		_frustums[_targetCamera].setRenderAABB(!aabb);
	}

	if (key == SDLK_0) {
		resetCameraPosition();
	}

	video::Camera& c = _renderCamera[_targetCamera];
	if (modifier & SDL_KMOD_SHIFT) {
		int delta = 0;
		if (key == SDLK_MINUS || key == SDLK_KP_MINUS) {
			delta = -1;
		} else if (key == SDLK_PLUS || key == SDLK_KP_PLUS) {
			delta = 1;
		}

		if (modifier & SDL_KMOD_CONTROL) {
			c.setNearPlane(c.nearPlane() + (float)delta);
		} else {
			c.setFarPlane(c.farPlane() + (float)delta);
		}
		if (delta != 0) {
			return true;
		}
	}

	return retVal;
}

TEST_APP(TestCamera)
