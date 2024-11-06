/**
 * @file
 */

#include "IMGUIApp.h"
#include "IconsLucide.h"
#include "Panel.h"
#include "Style.h"
#include "app/i18n/Language.h"
#include "imgui.h"
#include "imgui_internal.h"
#ifdef IMGUI_ENABLE_FREETYPE
#include "misc/freetype/imgui_freetype.h"
#endif

#include "command/Command.h"
#include "core/BindingContext.h"
#include "core/Color.h"
#include "core/Common.h"
#include "core/Log.h"
#include "core/StringUtil.h"
#include "core/TimeProvider.h"
#include "core/Var.h"
#include "dearimgui/backends/imgui_impl_opengl3.h"
#include "dearimgui/backends/imgui_impl_sdl3.h"
#include "dearimgui/implot.h"
#include "io/Filesystem.h"
#include "io/FormatDescription.h"
#include "util/KeybindingHandler.h"
#include "video/Renderer.h"
#include "video/TextureConfig.h"
#include "video/Types.h"

#ifdef IMGUI_ENABLE_TEST_ENGINE
#include "dearimgui/imgui_test_engine/imgui_te_ui.h"
#include "dearimgui/imgui_test_engine/imgui_te_engine.h"
#include "dearimgui/imgui_test_engine/imgui_te_context.h"
#include "dearimgui/imgui_test_engine/imgui_te_exporters.h"
#endif

#include "ArimoRegular.h"
#include "FileDialog.h"
#include "IMGUIEx.h"
#include "IMGUIStyle.h"

#include "FontLucide.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <glm/mat4x4.hpp>

namespace ui {

IMGUIApp::IMGUIApp(const io::FilesystemPtr &filesystem, const core::TimeProviderPtr &timeProvider,
				   size_t threadPoolSize)
	: Super(filesystem, timeProvider, threadPoolSize), _panels(), _fileDialog(this) {
}

IMGUIApp::~IMGUIApp() {
}

void IMGUIApp::onMouseMotion(void *windowHandle, int32_t x, int32_t y, int32_t relX, int32_t relY, int32_t mouseId) {
	Super::onMouseMotion(windowHandle, x, y, relX, relY, mouseId);

	SDL_Event ev{};
	ev.type = SDL_EVENT_MOUSE_MOTION;
	ev.motion.x = x;
	ev.motion.y = y;
	ev.motion.which = mouseId;
	ev.motion.windowID = SDL_GetWindowID((SDL_Window *)windowHandle);
	ImGui_ImplSDL3_ProcessEvent(&ev);
}

bool IMGUIApp::onMouseWheel(void *windowHandle, float x, float y, int32_t mouseId) {
	if (!Super::onMouseWheel(windowHandle, x, y, mouseId)) {
		SDL_Event ev{};
		ev.type = SDL_EVENT_MOUSE_WHEEL;
		ev.wheel.x = x;
		ev.wheel.y = y;
		ev.wheel.which = mouseId;
		ev.wheel.windowID = SDL_GetWindowID((SDL_Window *)windowHandle);

		ImGui_ImplSDL3_ProcessEvent(&ev);
	}
	return true;
}

void IMGUIApp::onMouseButtonRelease(void *windowHandle, int32_t x, int32_t y, uint8_t button, int32_t mouseId) {
	Super::onMouseButtonRelease(windowHandle, x, y, button, mouseId);
	SDL_Event ev{};
	ev.type = SDL_EVENT_MOUSE_BUTTON_UP;
	ev.button.button = button;
	ev.button.x = x;
	ev.button.y = y;
	ev.button.which = mouseId;
	ev.button.windowID = SDL_GetWindowID((SDL_Window *)windowHandle);
	ImGui_ImplSDL3_ProcessEvent(&ev);
}

void IMGUIApp::onMouseButtonPress(void *windowHandle, int32_t x, int32_t y, uint8_t button, uint8_t clicks, int32_t mouseId) {
	Super::onMouseButtonPress(windowHandle, x, y, button, clicks, mouseId);
	SDL_Event ev{};
	ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
	ev.button.button = button;
	ev.button.clicks = clicks;
	ev.button.x = x;
	ev.button.y = y;
	ev.button.which = mouseId;
	ev.button.windowID = SDL_GetWindowID((SDL_Window *)windowHandle);
	ImGui_ImplSDL3_ProcessEvent(&ev);
}

bool IMGUIApp::onTextInput(void *windowHandle, const core::String &text) {
	if (_showFileDialog) {
		_fileDialog.onTextInput(windowHandle, text);
	}
	SDL_Event ev{};
	ev.type = SDL_EVENT_TEXT_INPUT;
	ev.text.windowID = SDL_GetWindowID((SDL_Window *)windowHandle);
	ev.text.text = text.c_str();
	ImGui_ImplSDL3_ProcessEvent(&ev);
	return true;
}

bool IMGUIApp::onKeyPress(void *windowHandle, int32_t key, int16_t modifier) {
	if (!Super::onKeyPress(windowHandle, key, modifier) ||
		(core::bindingContext() == core::BindingContext::UI && key == SDLK_ESCAPE)) {
		SDL_Event ev{};
		ev.type = SDL_EVENT_KEY_DOWN;
		ev.key.scancode = (SDL_Scancode)SDL_SCANCODE_UNKNOWN;
		ev.key.key = (SDL_Keycode)key;
		ev.key.mod = modifier;
		ev.key.windowID = SDL_GetWindowID((SDL_Window *)windowHandle);
		ImGui_ImplSDL3_ProcessEvent(&ev);
		_keys.insert(key);
	}
	return true;
}

bool IMGUIApp::onKeyRelease(void *windowHandle, int32_t key, int16_t modifier) {
	if (!Super::onKeyRelease(windowHandle, key, modifier) || _keys.has(key)) {
		SDL_Event ev{};
		ev.type = SDL_EVENT_KEY_UP;
		ev.key.scancode = (SDL_Scancode)SDL_SCANCODE_UNKNOWN;
		ev.key.key = key;
		ev.key.mod = modifier;
		ev.key.windowID = SDL_GetWindowID((SDL_Window *)windowHandle);
		ImGui_ImplSDL3_ProcessEvent(&ev);
		_keys.remove(key);
	}
	return true;
}

bool IMGUIApp::handleSDLEvent(SDL_Event &event) {
	const bool state = Super::handleSDLEvent(event);
	if (event.type != SDL_EVENT_MOUSE_MOTION && event.type != SDL_EVENT_MOUSE_WHEEL && event.type != SDL_EVENT_MOUSE_BUTTON_UP &&
		event.type != SDL_EVENT_MOUSE_BUTTON_DOWN && event.type != SDL_EVENT_TEXT_INPUT && event.type != SDL_EVENT_KEY_UP &&
		event.type != SDL_EVENT_KEY_DOWN) {
		ImGui_ImplSDL3_ProcessEvent(&event);
	}
	return state;
}

app::AppState IMGUIApp::onConstruct() {
	_console.registerOutputCallbacks();
	const app::AppState state = Super::onConstruct();
	_console.construct();
	_fileDialog.construct();
	_lastDirectory = core::Var::getSafe(cfg::UILastDirectory);
	_languageVar = core::Var::getSafe(cfg::CoreLanguage);
	core::String uiStyleDefaultValue = core::string::toString(ImGui::StyleCorporateGrey);
	if (!isDarkMode()) {
		uiStyleDefaultValue = core::string::toString(ImGui::StyleLight);
	}
	_uistyle =
		core::Var::get(cfg::UIStyle, uiStyleDefaultValue.c_str(), _("Change the ui colors - [0-3]"), [](const core::String &val) {
			const int themeIdx = core::string::toInt(val);
			return themeIdx >= ImGui::StyleCorporateGrey && themeIdx < ImGui::MaxStyles;
		});
	core::Var::get(cfg::UINotifyDismissMillis, "3000", _("Timeout for notifications in millis"));
	core::Var::get(cfg::UIMultiMonitor, "true", _("Allow multi monitor setups - requires a restart"),
				   core::Var::boolValidator);
	_renderUI = core::Var::get(cfg::ClientRenderUI, "true", _("Render the ui"), core::Var::boolValidator);
#ifdef IMGUI_ENABLE_TEST_ENGINE
	_showMetrics = core::Var::get(cfg::UIShowMetrics, "true", core::CV_NOPERSIST, _("Show metric and debug window"),
								  core::Var::boolValidator);
#else
	_showMetrics = core::Var::get(cfg::UIShowMetrics, "false", core::CV_NOPERSIST, _("Show metric and debug window"),
								  core::Var::boolValidator);
#endif
	_uiFontSize =
		core::Var::get(cfg::UIFontSize, "14", -1, _("Allow to change the ui font size"), [](const core::String &val) {
			const float size = core::string::toFloat(val);
			return size >= 2.0f;
		});

	_uiKeyMap = core::Var::get(cfg::UIKeyMap, "0", _("Which keybinding to use"));
	core_assert(!_uiKeyMap->isDirty());

	command::Command::registerCommand("ui_showtextures",
									  [&](const command::CmdArgs &args) { _showTexturesDialog = true; });
	command::Command::registerCommand("ui_close", [&](const command::CmdArgs &args) { _closeModalPopup = true; });

	return state;
}

void IMGUIApp::loadFonts() {
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->Clear();

	ImFontGlyphRangesBuilder builder;
	builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
	const app::Language &lang = app::Language::fromName(_languageVar->strVal());
	if (lang.getLanguage() == "uk" || lang.getLanguage() == "ru") {
		builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
	} else if (lang.getLanguage() == "zh") {
		builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
	} else if (lang.getLanguage() == "jp") {
		builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
	} else if (lang.getLanguage() == "ko") {
		builder.AddRanges(io.Fonts->GetGlyphRangesKorean());
	} else if (lang.getLanguage() == "th") {
		builder.AddRanges(io.Fonts->GetGlyphRangesThai());
	} else if (lang.getLanguage() == "vi") {
		builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
	} else if (lang.getLanguage() == "el") {
		builder.AddRanges(io.Fonts->GetGlyphRangesGreek());
	}

	ImVector<ImWchar> glyphRanges;
	builder.BuildRanges(&glyphRanges);

	const float fontSize = _uiFontSize->floatVal();

	ImFontConfig fontCfg;
	fontCfg.MergeMode = true;
#ifdef IMGUI_ENABLE_FREETYPE
	fontCfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_LightHinting;
#endif

	ImFontConfig bigFontIconCfg;
	bigFontIconCfg.MergeMode = true;
	bigFontIconCfg.GlyphMinAdvanceX = fontSize * 2.0f;
	bigFontIconCfg.GlyphMaxAdvanceX = bigFontIconCfg.GlyphMinAdvanceX;
#ifdef IMGUI_ENABLE_FREETYPE
	bigFontIconCfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_LightHinting;
#endif

	ImFontConfig fontIconCfg;
	fontIconCfg.MergeMode = true;
	fontIconCfg.GlyphMinAdvanceX = fontSize;
	fontIconCfg.GlyphMaxAdvanceX = bigFontIconCfg.GlyphMinAdvanceX;

	static const ImWchar rangesLCIcons[] = {ICON_MIN_LC, ICON_MAX_LC, 0};

	_defaultFont = io.Fonts->AddFontFromMemoryCompressedTTF(ArimoRegular_compressed_data, ArimoRegular_compressed_size,
															fontSize, nullptr, glyphRanges.Data);
	io.Fonts->AddFontFromMemoryCompressedTTF(FontLucide_compressed_data, FontLucide_compressed_size, fontSize,
											 &fontIconCfg, rangesLCIcons);

	_bigFont = io.Fonts->AddFontFromMemoryCompressedTTF(ArimoRegular_compressed_data, ArimoRegular_compressed_size,
														fontSize * 2.0f, nullptr, glyphRanges.Data);

	_bigIconFont = io.Fonts->AddFontFromMemoryCompressedTTF(FontLucide_compressed_data, FontLucide_compressed_size,
															fontSize * 1.5f, &bigFontIconCfg, rangesLCIcons);

	_smallFont = io.Fonts->AddFontFromMemoryCompressedTTF(ArimoRegular_compressed_data, ArimoRegular_compressed_size,
														  fontSize * 0.8f, nullptr, glyphRanges.Data);
	io.Fonts->AddFontFromMemoryCompressedTTF(FontLucide_compressed_data, FontLucide_compressed_size, fontSize,
											 &fontIconCfg, rangesLCIcons);

	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	video::TextureConfig cfg;
	cfg.format(video::TextureFormat::RGBA);
	if (_texture == video::InvalidId) {
		_texture = video::genTexture(cfg);
	}

	video::bindTexture(video::TextureUnit::Upload, cfg.type(), _texture);
	video::setupTexture(cfg);
	video::uploadTexture(cfg.type(), cfg.format(), width, height, pixels, 0, cfg.samples());

	io.Fonts->TexID = (ImTextureID)(intptr_t)_texture;
}

static void *_imguiAlloc(size_t size, void *) {
	return core_malloc(size);
}

static void _imguiFree(void *mem, void *) {
	core_free(mem);
}

static void ImGui_ImplSDL3_NoShowWindow(ImGuiViewport *) {
}

app::AppState IMGUIApp::onInit() {
	const app::AppState state = Super::onInit();
	video::checkError();
	if (state != app::AppState::Running) {
		return state;
	}

	IMGUI_CHECKVERSION();
	ImGui::SetAllocatorFunctions(_imguiAlloc, _imguiFree);
	ImGui::CreateContext();
	ImPlot::CreateContext();

#ifdef IMGUI_ENABLE_TEST_ENGINE
	// Initialize Test Engine
	_imguiTestEngine = ImGuiTestEngine_CreateContext();
	ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(_imguiTestEngine);
	test_io.ConfigLogToTTY = _showWindow ? false : true;
	test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
	test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
#endif

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	if (!isSingleWindowMode() && core::Var::getSafe(cfg::UIMultiMonitor)->boolVal()) {
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		// io.ConfigViewportsNoAutoMerge = true;
		// io.ConfigViewportsNoTaskBarIcon = true;
	}
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	// io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
	// io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
	// test dpi related issues on linux with
	// xrandr | grep connected | grep -v disconnected | awk '{print $1}'
	// xrandr --output <screen-name> --scale 1.6x1.6

	if (_persistUISettings) {
		const core::String iniFile = core::string::format("%s-%i-imgui.ini", _appname.c_str(), _iniVersion);
		_writePathIni = _filesystem->homeWritePath(iniFile);
		io.IniFilename = _writePathIni.c_str();
	} else {
		io.IniFilename = nullptr;
	}
	const core::String logFile = _appname + "-imgui.log";
	_writePathLog = _filesystem->homeWritePath(logFile);
	io.LogFilename = _writePathLog.c_str();
	io.DisplaySize = _windowDimension;

	loadFonts();
	setColorTheme();
	_imguiBackendInitialized = ImGui_ImplSDL3_InitForOpenGL(_window, _rendererContext);
	ImGui_ImplOpenGL3_Init(nullptr);

	ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float);

	_console.init();

	Log::debug("Set up imgui");

#ifdef IMGUI_ENABLE_TEST_ENGINE
	if (registerUITests()) {
		ImGuiTestEngine_Start(_imguiTestEngine, ImGui::GetCurrentContext());
	}
#endif
	// if we decide to hide the window, we don't want docking to show externalized windows
	if (!_showWindow) {
		ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
		platform_io.Platform_ShowWindow = ImGui_ImplSDL3_NoShowWindow;
	}

	return state;
}

void IMGUIApp::setColorTheme() {
	// reset to default
	ImGuiStyle &style = ImGui::GetStyle();
	style = ImGuiStyle();

	switch (_uistyle->intVal()) {
	case ImGui::StyleCorporateGrey:
		ImGui::StyleColorsCorporateGrey();
		break;
	case ImGui::StyleDark:
		ImGui::StyleColorsDark();
		break;
	case ImGui::StyleLight:
		ImGui::StyleColorsLight();
		break;
	case ImGui::StyleClassic:
		ImGui::StyleColorsClassic();
		break;
	default:
		_uistyle->setVal(ImGui::StyleCorporateGrey);
		ImGui::StyleColorsCorporateGrey();
		break;
	}
	ImGui::StyleColorsNeoSequencer();
	ImGui::StyleImGuizmo();
}

const glm::vec4 &IMGUIApp::color(style::StyleColor color) {
	const int style = _uistyle->intVal();
	switch (color) {
	case style::ColorInactiveNode:
		return core::Color::Gray();
	case style::ColorGridBorder:
		if (style == ImGui::StyleLight) {
			return core::Color::DarkGray();
		}
		return core::Color::White();
	case style::ColorReferenceNode:
		return core::Color::LightGreen();
	case style::ColorHighlightArea: {
		static const glm::vec4 c = core::Color::alpha(core::Color::Green(), 0.2f);
		return c;
	}
	case style::ColorUVEditor: {
		if (style == ImGui::StyleLight || style == ImGui::StyleClassic) {
			return core::Color::DarkRed();
		}
		return core::Color::LightRed();
	}
	case style::ColorActiveNode:
		if (style == ImGui::StyleLight || style == ImGui::StyleClassic) {
			return core::Color::DarkGreen();
		}
		return core::Color::White();
	case style::ColorBone:
		return core::Color::LightGray();
	}
	return core::Color::White();
}

void IMGUIApp::beforeUI() {
}

void IMGUIApp::renderBindingsDialog() {
	if (ImGui::Begin(_("Bindings"), &_showBindingsDialog, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Modal)) {
		const util::BindMap &bindings = _keybindingHandler.bindings();
		static const uint32_t TableFlags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable |
										   ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersInner |
										   ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
		const ImVec2 outerSize(0.0f, 400.0f);
		if (ImGui::BeginTable("##bindingslist", 4, TableFlags, outerSize)) {
			ImGui::TableSetupColumn(_("Keys"), ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn(_("Command"), ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn(_("Context"), ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn(_("Description"), ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			int n = 0;
			for (util::BindMap::const_iterator i = bindings.begin(); i != bindings.end(); ++i) {
				const util::CommandModifierPair &pair = i->second;
				const core::String &command = pair.command;
				const core::String &keyBinding =
					util::KeyBindingHandler::toString(i->first, i->second.modifier, pair.count);
				const command::Command *cmd = nullptr;
				if (command.contains(" ")) {
					cmd = command::Command::getCommand(command.substr(0, command.find(" ")));
				} else {
					cmd = command::Command::getCommand(command);
				}
				if (_bindingsFilter.size() >= 2u) {
					const bool matchCmd = core::string::icontains(command, _bindingsFilter);
					const bool matchKey = core::string::icontains(keyBinding, _bindingsFilter);
					const bool matchHelp = cmd ? core::string::icontains(cmd->help(), _bindingsFilter) : true;
					if (!matchCmd && !matchKey && !matchHelp) {
						continue;
					}
				}
				ImGui::TableNextColumn();
				// TODO: change binding
				const core::String &deleteButton = core::string::format(ICON_LC_TRASH "##del-key-%i", n++);
				if (ImGui::Button(deleteButton.c_str())) {
					command::executeCommands(core::string::format("unbind \"%s\"", keyBinding.c_str()),
											 &_lastExecutedCommand);
				}
				ImGui::SameLine();
				ImGui::TextUnformatted(keyBinding.c_str());
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(command.c_str());
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(core::bindingContextString(pair.context).c_str());
				ImGui::TableNextColumn();
				if (!cmd) {
					ImGui::TextColored(core::Color::Red(), _("Failed to get command for %s"), command.c_str());
				} else {
					ImGui::TextUnformatted(cmd->help().c_str());
				}
			}
			ImGui::EndTable();
		}
		if (!_uiKeyMaps.empty()) {
			keyMapOption();
		} else {
			if (ImGui::Button(_("Reset to default"))) {
				_resetKeybindings = true;
			}
		}
		ImGui::SameLine();
		ImGui::InputText(_("Filter"), &_bindingsFilter);
	}
	ImGui::End();
}

void IMGUIApp::renderCvarDialog() {
	if (ImGui::Begin(_("Configuration variables"), &_showCvarDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
		static const uint32_t TableFlags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable |
										   ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersInner |
										   ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
		const ImVec2 outerSize(0.0f, 400.0f);
		if (ImGui::BeginTable("##cvars", 4, TableFlags, outerSize)) {
			ImGui::TableSetupColumn(_("Name"), ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn(_("Value"), ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("##reset", ImGuiTableFlags_SizingFixedFit);
			ImGui::TableSetupColumn(_("Description"), ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			core::Var::visit([](const core::VarPtr &var) {
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(var->name().c_str());
				ImGui::TableNextColumn();
				const bool readOnly = var->getFlags() & core::CV_READONLY;
				ImGui::BeginDisabled(readOnly);
				const core::String type = "##" + var->name();
				if (var->typeIsBool()) {
					bool value = var->boolVal();
					if (ImGui::Checkbox(type.c_str(), &value)) {
						var->setVal(value);
					}
				} else {
					int flags = 0;
					const bool secret = var->getFlags() & core::CV_SECRET;
					if (secret) {
						flags |= ImGuiInputTextFlags_Password;
					}
					core::String value = var->strVal();
					if (ImGui::InputText(type.c_str(), &value, flags)) {
						var->setVal(value);
					}
				}
				ImGui::EndDisabled();
				ImGui::TableNextColumn();
				if (!readOnly) {
					if (ImGui::Button(_("Reset"))) {
						var->reset();
					}
					ImGui::TooltipTextUnformatted(_("Reset to default value"));
				}
				ImGui::TableNextColumn();
				ImGui::Text("%s", var->help() ? var->help() : "");
			});
			ImGui::EndTable();
		}
		if (ImGui::Button(_("Close"))) {
			_showCvarDialog = false;
		}
	}
	ImGui::End();
}

void IMGUIApp::renderCommandDialog() {
	if (ImGui::Begin(_("Commands"), &_showCommandDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
		static const uint32_t TableFlags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable |
										   ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersInner |
										   ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
		const ImVec2 outerSize(0.0f, 400.0f);
		if (ImGui::BeginTable("##commands", 2, TableFlags, outerSize)) {
			ImGui::TableSetupColumn(_("Name"), ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn(_("Description"), ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			command::Command::visitSorted([](const command::Command &cmd) {
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(cmd.name().c_str());
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(cmd.help().c_str());
			});
			ImGui::EndTable();
		}
		if (ImGui::Button(_("Close"))) {
			_showCommandDialog = false;
		}
	}
	ImGui::End();
}

void IMGUIApp::renderTexturesDialog() {
	if (ImGui::Begin(_("Textures"), &_showTexturesDialog)) {
		const core::DynamicSet<video::Id> &textures = video::textures();
		const ImVec2 size(512, 512);
		int textureCnt = 0;
		for (const auto &e : textures) {
			ImGui::Image(e->first, size, ImVec2(), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			// TODO GL_INVALID_OPERATION error generated. Target doesn't match the texture's target.
			++textureCnt;
			if (textureCnt % 2) {
				ImGui::SameLine();
			}
		}
	}
	ImGui::End();
}

static void localizeRegisterEntry(ImGuiLocKey key, const char* text) {
	const ImGuiLocEntry entry{key, text};
	ImGui::LocalizeRegisterEntries(&entry, 1);
}

app::AppState IMGUIApp::onRunning() {
	core_trace_scoped(IMGUIAppOnRunning);
	app::AppState state = Super::onRunning();

	if (state != app::AppState::Running) {
		return state;
	}
	video::clear(video::ClearFlag::Color);

	_console.update(_deltaFrameSeconds);

	if (_uiKeyMap->isDirty() || _resetKeybindings) {
		_keybindingHandler.clear();
		loadKeymap(_uiKeyMap->intVal());
		_uiKeyMap->markClean();
		_resetKeybindings = false;
	}

	if (_languageVar->isDirty()) {
		// IMPORTANT: ###xxx suffixes must be same in ALL languages to allow for automation.
		localizeRegisterEntry(ImGuiLocKey_TableSizeOne, _("Size column to fit###SizeOne"));
		localizeRegisterEntry(ImGuiLocKey_TableSizeAllFit, _("Size all columns to fit###SizeAll"));
		localizeRegisterEntry(ImGuiLocKey_TableSizeAllDefault, _("Size all columns to default###SizeAll"));
		localizeRegisterEntry(ImGuiLocKey_TableResetOrder, _("Reset order###ResetOrder"));
		localizeRegisterEntry(ImGuiLocKey_WindowingMainMenuBar, _("(Main menu bar)"));
		localizeRegisterEntry(ImGuiLocKey_WindowingPopup, _("(Popup)"));
		localizeRegisterEntry(ImGuiLocKey_WindowingUntitled, _("(Untitled)"));
		localizeRegisterEntry(ImGuiLocKey_OpenLink_s, _("Open '%s'"));
		localizeRegisterEntry(ImGuiLocKey_CopyLink, _("Copy Link###CopyLink"));
		localizeRegisterEntry(ImGuiLocKey_DockingHideTabBar, _("Hide tab bar###HideTabBar"));
		localizeRegisterEntry(ImGuiLocKey_DockingHoldShiftToDock, _("Hold SHIFT to enable Docking window."));
		localizeRegisterEntry(ImGuiLocKey_DockingDragToUndockOrMoveNode,
							  _("Click and drag to move or undock whole node."));
		static_assert(ImGuiLocKey_COUNT == 13, "Please update ImGui translations");
	}

	const float dpiScale = core_max(0.1f, ImGui::GetMainViewport()->DpiScale);
	if (_languageVar->isDirty() || _uiFontSize->isDirty() || glm::abs(dpiScale - _dpiScale) > 0.001f) {
		_dpiScale = dpiScale;
		loadFonts();
		_uiFontSize->markClean();
		_languageVar->markClean();
	}

	if (_uistyle->isDirty()) {
		setColorTheme();
		_uistyle->markClean();
	}

	{
		core_trace_scoped(IMGUIAppBeforeUI);
		beforeUI();
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	const bool renderUI = _renderUI->boolVal();
	if (renderUI) {
		core_trace_scoped(IMGUIAppOnRenderUI);
		onRenderUI();
		if (_showConsole) {
			_console.render(_lastExecutedCommand);
		}

#ifdef IMGUI_ENABLE_TEST_ENGINE
		if (_showWindow) {
			ImGuiTestEngine_ShowTestEngineWindows(_imguiTestEngine, nullptr);
		}
#endif

		if (_closeModalPopup) {
			if (ImGui::GetTopMostPopupModal() != nullptr) {
				GImGui->OpenPopupStack.resize(GImGui->OpenPopupStack.size() - 1);
			}
			_closeModalPopup = false;
		}

		if (_showTexturesDialog) {
			renderTexturesDialog();
		}

		if (_showCommandDialog) {
			renderCommandDialog();
		}

		if (_showCvarDialog) {
			renderCvarDialog();
		}

		if (_showBindingsDialog) {
			renderBindingsDialog();
		}

		bool showMetrics = _showMetrics->boolVal();
		if (showMetrics) {
			ImGui::ShowMetricsWindow(&showMetrics);
			if (!showMetrics) {
				_showMetrics->setVal("false");
			}
		}
		_console.renderNotifications();

		core::String buf;
		const io::FormatDescription *formatDesc = nullptr;
		if (_showFileDialog && _fileDialog.showFileDialog(_fileDialogOptions, buf, _fileDialogMode, &formatDesc)) {
			if (buf[0] != '\0') {
				_fileDialogCallback(buf, formatDesc);
			}
			_showFileDialog = false;
		}

		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
			core::setBindingContext(core::BindingContext::UI);
		} else {
			core::setBindingContext(core::BindingContext::All);
		}
	} else {
		core::setBindingContext(core::BindingContext::All);
	}

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		video::activateContext(_window, _rendererContext);
	}

#ifdef IMGUI_ENABLE_TEST_ENGINE
	ImGuiTestEngine_PostSwap(_imguiTestEngine);
#endif

	video::scissor(0, 0, _frameBufferDimension.x, _frameBufferDimension.y);

#ifdef IMGUI_ENABLE_TEST_ENGINE
	// run the tests in command line mode if the window is hidden
	if (!_showWindow) {
		// delay the execution of the tests a few frames to allow all tests to be registered by the app
		if (_startedFromCommandlineFrameDelay > 0) {
			_startedFromCommandlineFrameDelay--;
		}
		if (_startedFromCommandlineFrameDelay == 0) {
			ImGuiTestEngine_QueueTests(_imguiTestEngine, ImGuiTestGroup_Tests, "tests", ImGuiTestRunFlags_RunFromCommandLine);
			_startedFromCommandlineFrameDelay--;
		}

		if (_startedFromCommandlineFrameDelay == -1) {
			if (ImGuiTestEngine_IsTestQueueEmpty(_imguiTestEngine)) {
				int testedCnt = 0;
				int successCnt = 0;
				core::String testresults = _appname + "-ui-tests.xml";
				ImGuiTestEngine_ExportEx(_imguiTestEngine, ImGuiTestEngineExportFormat_JUnitXml, testresults.c_str());
				ImGuiTestEngine_GetResult(_imguiTestEngine, testedCnt, successCnt);
				if (successCnt != testedCnt) {
					_exitCode = 1;
				}
				requestQuit();
			}
		}
	}
#endif

	return app::AppState::Running;
}

void IMGUIApp::loadKeymap(int keymap) {
	_keybindingHandler.registerBinding("escape", "ui_close", "ui");
}

void IMGUIApp::languageOption() {
	core::String currentLanguage = _languageVar->strVal();
	if (ImGui::BeginCombo(_("Language"), _languageVar->strVal().c_str())) {
		const core::DynamicArray<app::Language> &languages = _dictManager.getLanguages();
		app::Language currentLang = app::Language::fromEnv(currentLanguage);
		for (const auto &lang : languages) {
			bool isSelected = lang == currentLang;
			if (ImGui::Selectable(lang.str().c_str(), isSelected)) {
				_languageVar->setVal(lang.str());
				setLanguage(lang.str());
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::TooltipTextUnformatted(_("To change the language everywhere restart the application"));
}

bool IMGUIApp::keyMapOption() {
	if (_uiKeyMaps.empty()) {
		return false;
	}
	const int currentKeyMap = _uiKeyMap->intVal();
	if (ImGui::BeginCombo(_("Keymap"), _uiKeyMaps[(int)currentKeyMap].c_str(), ImGuiComboFlags_None)) {
		for (int i = 0; i < (int)_uiKeyMaps.size(); ++i) {
			const bool selected = i == currentKeyMap;
			if (ImGui::Selectable(_uiKeyMaps[i].c_str(), selected)) {
				_uiKeyMap->setVal(core::string::toString(i));
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	return true;
}

void IMGUIApp::colorReductionOptions() {
	const core::VarPtr &colorReduction = core::Var::getSafe(cfg::CoreColorReduction);
	if (ImGui::BeginCombo(_("Color reduction"), colorReduction->strVal().c_str(), ImGuiComboFlags_None)) {
		core::Color::ColorReductionType type = core::Color::toColorReductionType(colorReduction->strVal().c_str());
		for (int i = 0; i < (int)core::Color::ColorReductionType::Max; ++i) {
			const bool selected = i == (int)type;
			const char *str = core::Color::toColorReductionTypeString((core::Color::ColorReductionType)i);
			if (ImGui::Selectable(str, selected)) {
				colorReduction->setVal(str);
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::TooltipTextUnformatted(
		_("The color reduction algorithm that is used when importing RGBA colors from images or rgba formats"));
}

app::AppState IMGUIApp::onCleanup() {
#ifdef IMGUI_ENABLE_TEST_ENGINE
	ImGuiTestEngine_Stop(_imguiTestEngine);
	_fileDialog.unregisterUITests(_imguiTestEngine);
#endif
	if (_imguiBackendInitialized) {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		_imguiBackendInitialized = false;
	}
	if (ImGui::GetCurrentContext() != nullptr) {
		ImPlot::DestroyContext();
		ImGui::DestroyPlatformWindows();
		ImGui::DestroyContext();
	}
#ifdef IMGUI_ENABLE_TEST_ENGINE
	ImGuiTestEngine_DestroyContext(_imguiTestEngine);
	_imguiTestEngine = nullptr;
#endif
	_console.shutdown();
	return Super::onCleanup();
}

void IMGUIApp::fileDialog(const video::FileDialogSelectionCallback &callback, const video::FileDialogOptions &options,
						  video::OpenFileMode mode, const io::FormatDescription *formats,
						  const core::String &filename) {
	_showFileDialog = true;
	_fileDialogCallback = callback;
	_fileDialogOptions = options;
	_fileDialogMode = mode;
	_fileDialog.openDir(mode, formats, filename);
}

void IMGUIApp::addPanel(Panel *panel) {
	core_assert(panel != nullptr);
	_panels.push_back(panel);
}

void IMGUIApp::removePanel(Panel *panel) {
	auto iter = core::find(_panels.begin(), _panels.end(), panel);
	if (iter != _panels.end()) {
		_panels.erase(iter);
	}
}

Panel* IMGUIApp::getPanel(const core::String &title) {
	for (Panel *panel : _panels) {
		if (panel->_title == title) {
			return panel;
		}
	}
	Log::error("Panel not found - available are:");
	for (Panel *panel : _panels) {
		Log::error("- %s", panel->_title.c_str());
	}
	return nullptr;
}

} // namespace ui
