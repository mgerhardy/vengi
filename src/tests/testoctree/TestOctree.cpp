/**
 * @file
 */
#include "TestOctree.h"
#include "core/Color.h"
#include "video/ScopedLineWidth.h"
#include "core/collection/Array.h"
#include "core/StringUtil.h"
#include "testcore/TestAppMain.h"
#include "core/Log.h"
#include <SDL3/SDL.h>
#include <glm/gtc/type_ptr.hpp>

TestOctree::TestOctree(const io::FilesystemPtr& filesystem, const core::TimeProviderPtr& timeProvider) :
		Super(filesystem, timeProvider) {
	init(ORGANISATION, "testoctree");
	setRenderAxis(true);
}

bool TestOctree::onKeyPress(void *windowHandle, int32_t key, int16_t modifier) {
	if (Super::onKeyPress(windowHandle, key, modifier)) {
		return true;
	}
	const bool current = isRelativeMouseMode();
	if (current) {
		if (key == SDLK_PLUS || key == SDLK_KP_PLUS || key == SDLK_INSERT) {
			insert();
		} else if (key == SDLK_DELETE || key == SDLK_KP_CLEAR) {
			clear();
		}
	}
	return true;
}

void TestOctree::insert() {
	const math::AABB<int>& aabb = _octree.aabb();
	const glm::ivec3& mins = aabb.mins();
	const glm::ivec3& maxs = aabb.maxs();
	glm::ivec3 pos;
	for (int i = 0; i < (int)decltype(pos)::length(); ++i) {
		pos[i] = _random.random(mins[i] + 1, maxs[i] - 1);
	}
	if (_octree.insert(Wrapper(pos))) {
		_dirty = true;
	} else {
		Log::info("Failed to add element for %i:%i:%i", pos.x, pos.y, pos.z);
	}
}

void TestOctree::clear() {
	_octree.clear();
	_results.clear();
	_dirty = true;
}

app::AppState TestOctree::onInit() {
	app::AppState state = Super::onInit();
	if (state != app::AppState::Running) {
		return state;
	}

	if (!_shapeRenderer.init()) {
		Log::error("Failed to init the shape renderer");
		return app::AppState::InitFailure;
	}

	const glm::ivec3& center = _octree.aabb().getCenter();
	_queryMins = center - 150;
	_queryMaxs = center + 150;

	_octree.setListener(&_listener);
	camera().setFarPlane(4000.0f);
	camera().setWorldPosition(glm::vec3(0.0f, 1250.0f, 2500.0f));

	insert();

	return state;
}

void TestOctree::Listener::onNodeCreated(const Node& parent, const Node& child) const {
	Log::info("Created node");
}

void TestOctree::handleDirtyState() {
	if (!_dirty) {
		return;
	}
	_dirty = false;

	_nodes = 0;
	_itemIndex = -1;
	_itemVector.clear();
	_octree.visit([&] (const Node& node) {
		++_nodes;
		const math::AABB<int>& aabb = node.aabb();
		const int depth = node.depth();
		_itemVector.push_back(aabb);
		Log::info("aabb for depth %i: %i:%i:%i",
				depth, aabb.getWidth().x, aabb.getWidth().y, aabb.getWidth().z);
	});

	// build AABBs
	_octree.visit([this] (const Node& node) {
		static const core::Array<glm::vec4, 5> colors {
			core::Color::Blue(),
			core::Color::Red(),
			core::Color::Green(),
			core::Color::Yellow(),
			core::Color::Cyan()
		};
		_shapeBuilder.setColor(colors[node.depth() % colors.size()]);
		_shapeBuilder.aabb(node.aabb());
	});
	_shapeRenderer.createOrUpdate(_aabbMeshes, _shapeBuilder);
	_shapeBuilder.clear();

	// build spheres
	_octree.visit([this] (const Node& node) {
		const Tree::Contents& contents = node.getContents();
		for (const Wrapper& wrapper : contents) {
			const math::AABB<int>& itemAABB = wrapper.aabb();
			_shapeBuilder.setPosition(itemAABB.getCenter());
			_shapeBuilder.sphere(10, 10, 5.0f);
		}
	});
	_shapeRenderer.createOrUpdate(_itemMeshes, _shapeBuilder);
	_shapeBuilder.clear();
}

void TestOctree::onRenderUI() {
	handleDirtyState();
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(400, 120), ImGuiCond_FirstUseEver);
	ImGui::Begin("Keys and information");
	ImGui::BulletText("+/INSERT: Insert new element");
	ImGui::BulletText("DELETE: Remove all elements");
	Super::onRenderUI();
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 260), ImGuiCond_FirstUseEver);
	ImGui::Begin("Actions");
	if (ImGui::Button("Clear")) {
		clear();
	}
	ImGui::SameLine();
	if (ImGui::Button("Random Insert")) {
		insert();
	}
	ImGui::Separator();
	ImGui::Checkbox("Render AABBs", &_renderAABBs);
	ImGui::Checkbox("Render Items", &_renderItems);
	ImGui::Separator();

	ImGui::InputInt3("mins", (int*)glm::value_ptr(_queryMins));
	ImGui::InputInt3("maxs", (int*)glm::value_ptr(_queryMaxs));

	if (ImGui::Button("Query")) {
		_results.clear();
		_queryAABB = math::AABB<int>(_queryMins, _queryMaxs);
		_octree.query(_queryAABB, _results);
		Log::info("Query (%i:%i:%i) to (%i:%i:%i) (found: %i)",
				_queryMins.x, _queryMins.y, _queryMins.z, _queryMaxs.x, _queryMaxs.y, _queryMaxs.z, (int)_results.size());
	}
	_shapeBuilder.clear();
	_shapeBuilder.setColor(core::Color::White());
	_shapeBuilder.aabb(_queryAABB);
	_shapeRenderer.createOrUpdate(_queryMeshes, _shapeBuilder);
	_shapeBuilder.clear();
	const glm::ivec3& mins = _octree.aabb().mins();
	const glm::ivec3& maxs = _octree.aabb().maxs();
	ImGui::Text("Tree size: mins(%i:%i:%i) maxs(%i:%i:%i)",
			mins.x, mins.y, mins.z, maxs.x, maxs.y, maxs.z);
	ImGui::BulletText("Nodes: %i", _nodes);
	ImGui::BulletText("Elements: %i", _octree.count());
	ImGui::Separator();
	ImGui::BulletText("Results: %i", (int)_results.size());
	ImGui::PushItemWidth(400);

	typedef struct _VectorGetter {
		struct AABBInfo {
			math::AABB<int> aabb;
			core::String info;
		};
		std::vector<AABBInfo> _itemVector;
		_VectorGetter(const std::vector<math::AABB<int> >& itemVector) {
			for (const auto& aabb : itemVector) {
				const glm::ivec3& mins = aabb.mins();
				const glm::ivec3& maxs = aabb.maxs();
				_itemVector.emplace_back(AABBInfo{aabb,
						core::string::format("mins(%i:%i:%i) maxs(%i:%i:%i)",
						mins.x, mins.y, mins.z, maxs.x, maxs.y, maxs.z)});
			}
		}
		static const char* resolve(void* pmds, int idx) {
			const _VectorGetter& mds = *((const _VectorGetter*) pmds);
			if (idx < 0 || idx > (int)mds._itemVector.size()) {
				return nullptr;
			}
			return mds._itemVector[idx].info.c_str();
		}
	} VectorGetter;
	VectorGetter mds(_itemVector);
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
	const int numEntries = (int)_itemVector.size();
	ImGui::Combo("Nodes", &_itemIndex, &VectorGetter::resolve, &mds,
			numEntries, glm::clamp(numEntries, 0, 25));
	ImGui::PopItemWidth();
	if (_itemIndex != -1) {
		const math::AABB<int>& selected = _itemVector[_itemIndex];
		_queryMins = selected.mins();
		_queryMaxs = selected.maxs();
	}
	ImGui::End();
}

void TestOctree::doRender() {
	if (_renderAABBs) {
		_shapeRenderer.render(_aabbMeshes, camera());
	}
	if (_renderItems) {
		_shapeRenderer.render(_itemMeshes, camera());
	}
	video::ScopedLineWidth lineWidth(2.0f);
	_shapeRenderer.render(_queryMeshes, camera());
}

app::AppState TestOctree::onCleanup() {
	_shapeRenderer.shutdown();
	return Super::onCleanup();
}

TEST_APP(TestOctree)
