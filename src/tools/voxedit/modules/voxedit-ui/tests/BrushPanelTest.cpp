/**
 * @file
 */

#include "../BrushPanel.h"
#include "voxedit-util/SceneManager.h"

namespace voxedit {

void BrushPanel::registerUITests(ImGuiTestEngine *engine, const char *title) {
	ImGuiTest *test = IM_REGISTER_TEST(engine, testCategory(), testName());
	test->TestFunc = [=](ImGuiTestContext *ctx) {
		voxedit::ModifierFacade &modifier = _sceneMgr->modifier();
		ctx->ItemClick("//###viewport1");
		ctx->SetRef(title);
		focusWindow(ctx, title);
		// enable edit mode - TODO: find the correct viewport id and don't hardcode it
		for (int i = 0; i < (int)BrushType::Max; ++i) {
			const core::String id = core::String::format("###button%d", i);
			ctx->ItemClick(id.c_str());
			const BrushType brushType = modifier.brushType();
			IM_CHECK_EQ((int)brushType, i);
		}
	};
}

} // namespace voxedit
