/**
 * @file
 */

#include "IMGUIEx.h"
#include "IconsLucide.h"
#include "app/App.h"
#include "PopupAbout.h"
#include "engine-config.h"

namespace ui {

void metricOption() {
	const core::VarPtr &metricFlavor = core::Var::getSafe(cfg::MetricFlavor);
	bool metrics = !metricFlavor->strVal().empty();
	if (ImGui::Checkbox(_("Enable sending anonymous metrics"), &metrics)) {
		if (metrics) {
			metricFlavor->setVal("json");
		} else {
			metricFlavor->setVal("");
		}
	}
	ImGui::TooltipTextUnformatted(_("Send anonymous usage statistics"));
}

void popupAbout(const std::function<void()> &customTabs, bool isNewVersionAvailable) {
	const int popupWidth = 600;
	const int popupHeight = 400;
	ImGui::SetNextWindowSize(ImVec2(popupWidth, popupHeight), ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal(POPUP_TITLE_ABOUT)) {
		if (ImGui::BeginChild("##scrollwindow", ImVec2(popupWidth, popupHeight - 80))) {
			if (ImGui::BeginTabBar("##abouttabbar")) {
				const float urlIconWidth = ImGui::GetContentRegionAvail().x;

				if (ImGui::BeginTabItem(app::App::getInstance()->fullAppname().c_str())) {
					ImGui::Text("%s " PROJECT_VERSION, app::App::getInstance()->appname().c_str());
					ImGui::Dummy(ImVec2(1, 10));
					ImGui::TextUnformatted(_("This is a beta release!"));
					if (isNewVersionAvailable) {
						ImGui::TextUnformatted(_("A new version is available!"));
					} else {
						ImGui::TextUnformatted(_("You are using the latest version."));
					}
					metricOption();

					ImGui::Dummy(ImVec2(1, 10));
					ImGui::URLIconItem(ICON_LC_GITHUB, _("Bug reports"), "https://github.com/vengi-voxel/vengi/issues", urlIconWidth);
					ImGui::URLIconItem(ICON_LC_CIRCLE_HELP, _("Help"), "https://vengi-voxel.github.io/vengi/", urlIconWidth);
					ImGui::URLIconItem(ICON_LC_TWITTER, _("Twitter"), "https://twitter.com/MartinGerhardy", urlIconWidth);
					ImGui::URLIconItem(ICON_LC_SQUARE, _("Mastodon"), "https://mastodon.social/@mgerhardy", urlIconWidth);
					ImGui::URLIconItem(ICON_LC_SQUARE, _("Discord"), "https://vengi-voxel.de/discord", urlIconWidth);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem(_("Credits"))) {
					ImGui::URLItem("backward-cpp", "https://github.com/bombela/backward-cpp", urlIconWidth);
#ifdef USE_CURL
					ImGui::Text("libCURL");
#endif
					ImGui::URLItem("cute_aseprite", "https://github.com/RandyGaul/cute_headers", urlIconWidth);
					ImGui::URLItem("dearimgui", "https://github.com/ocornut/imgui", urlIconWidth);
					ImGui::URLItem("glm", "https://github.com/g-truc/glm", urlIconWidth);
					ImGui::URLItem("IconFontCppHeaders", "https://github.com/juliettef/IconFontCppHeaders", urlIconWidth);
					ImGui::URLItem("imguizmo", "https://github.com/CedricGuillemet/ImGuizmo", urlIconWidth);
					ImGui::URLItem("im-neo-sequencer", "https://gitlab.com/GroGy/im-neo-sequencer", urlIconWidth);
					ImGui::URLItem("implot", "https://github.com/epezent/implot", urlIconWidth);
					ImGui::URLItem("libvxl", "https://github.com/xtreme8000/libvxl", urlIconWidth);
#ifdef USE_LIBJPEG
					ImGui::URLItem("libjpeg", "https://github.com/libjpeg-turbo/libjpeg-turbo", urlIconWidth);
#endif
					ImGui::URLItem("lua", "https://www.lua.org/", urlIconWidth);
					ImGui::URLItem("lucide", "https://lucide.dev/", urlIconWidth);
					ImGui::URLItem("meshoptimizer", "https://github.com/zeux/meshoptimizer", urlIconWidth);
					ImGui::URLItem("ogt_vox", "https://github.com/jpaver/opengametools", urlIconWidth);
					ImGui::URLItem("polyvox", "http://www.volumesoffun.com/", urlIconWidth);
					ImGui::URLItem("SDL3", "https://github.com/libsdl-org/SDL", urlIconWidth);
					ImGui::URLItem("stb/SOIL2", "https://github.com/SpartanJ/SOIL2", urlIconWidth);
					ImGui::URLItem("tinygettext", "https://github.com/tinygettext/tinygettext/", urlIconWidth);
					ImGui::URLItem("tinygltf", "https://github.com/syoyo/tinygltf", urlIconWidth);
					ImGui::URLItem("tinyobjloader", "https://github.com/tinyobjloader/tinyobjloader", urlIconWidth);
					ImGui::URLItem("ufbx", "https://github.com/bqqbarbhg/ufbx", urlIconWidth);
					ImGui::URLItem("Yocto/GL", "https://github.com/xelatihy/yocto-gl", urlIconWidth);
#ifdef USE_ZLIB
					ImGui::Text("zlib");
#endif
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem(_("Paths"))) {
					for (const core::String &path : io::filesystem()->registeredPaths()) {
						const core::String &abspath = io::filesystem()->sysAbsolutePath(path);
						if (abspath.empty()) {
							continue;
						}
						core::String fileurl = "file://" + abspath;
						ImGui::URLItem(abspath.c_str(), fileurl.c_str(), urlIconWidth);
					}
					ImGui::EndTabItem();
				}
				if (customTabs) {
					customTabs();
				}

				ImGui::EndTabBar();
			}
		}
		ImGui::EndChild();
		ImGui::Separator();
		if (ImGui::IconButton(ICON_LC_CHECK, _("Close"))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}
}

} // namespace ui
