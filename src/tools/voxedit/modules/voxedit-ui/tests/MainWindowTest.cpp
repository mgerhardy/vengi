/**
 * @file
 */

#include "../MainWindow.h"
#include "voxedit-util/SceneManager.h"

namespace voxedit {

void MainWindow::registerUITests(ImGuiTestEngine *engine, const char *title) {
	ImGuiTest *test = IM_REGISTER_TEST(engine, testCategory(), testName());
	test->TestFunc = [=](ImGuiTestContext *ctx) {
		ctx->SetRef(title);
		focusWindow(ctx, title);
	};
}

} // namespace voxedit
