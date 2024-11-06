set(LIB ui)

set(SRCS
	FileDialog.cpp FileDialog.h
	IconsLucide.h
	IMGUIConsole.cpp IMGUIConsole.h
	IMGUIEx.cpp IMGUIEx.h
	IMGUIApp.cpp IMGUIApp.h
	IMGUIStyle.cpp IMGUIStyle.h
	Notify.cpp Notify.h
	Panel.cpp Panel.h
	PopupAbout.h PopupAbout.cpp
	ScopedStyle.h
	Style.cpp Style.h
	TextEditor.h TextEditor.cpp
	Toolbar.h Toolbar.cpp
	DragAndDropPayload.h

	dearimgui/imgui.cpp dearimgui/imgui_draw.cpp
	dearimgui/imgui_widgets.cpp
	dearimgui/imgui_tables.cpp
	dearimgui/imgui_stdlib.cpp dearimgui/imgui_stdlib.h
	dearimgui/imgui.h
	dearimgui/imgui_internal.h
	dearimgui/imconfig.h
	dearimgui/imstb_rectpack.h
	dearimgui/imstb_textedit.h
	dearimgui/imstb_truetype.h

	dearimgui/backends/imgui_impl_opengl3.cpp dearimgui/backends/imgui_impl_opengl3.h
	dearimgui/backends/imgui_impl_sdl3.cpp dearimgui/backends/imgui_impl_sdl3.h

	dearimgui/imgui_neo_sequencer.cpp dearimgui/imgui_neo_sequencer.h
	dearimgui/imgui_neo_internal.cpp dearimgui/imgui_neo_internal.h

	dearimgui/implot.cpp dearimgui/implot.h
	dearimgui/implot_items.cpp dearimgui/implot_internal.h

	dearimgui/ImGuizmo.cpp dearimgui/ImGuizmo.h
)

set(DEFINITIONS -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS -DIMGUI_DISABLE_OBSOLETE_KEYIO)
set(LIBS)

if (USE_OPENGLES)
	list(APPEND DEFINITIONS -DIMGUI_IMPL_OPENGL_ES2)
endif()

find_package(Freetype)
if (FREETYPE_FOUND)
	list(APPEND SRCS dearimgui/misc/freetype/imgui_freetype.cpp)
	list(APPEND DEFINITIONS -DIMGUI_ENABLE_FREETYPE)
	list(APPEND LIBS Freetype::Freetype)
endif()

engine_add_module(TARGET ${LIB} SRCS ${SRCS} DEPENDENCIES render app ${LIBS})
target_compile_definitions(${LIB} PUBLIC ${DEFINITIONS})

engine_generated_library(ui-fonts)
engine_compressed_file_to_header(ui-fonts FontLucide ${ROOT_DIR}/data/ui/lucide.ttf ${GENERATE_DIR}/ui/FontLucide.h)
engine_compressed_file_to_header(ui-fonts ArimoRegular ${ROOT_DIR}/data/ui/Arimo-Regular.ttf ${GENERATE_DIR}/ui/ArimoRegular.h)
target_link_libraries(${LIB} PRIVATE ui-fonts)
target_include_directories(${LIB} PUBLIC . dearimgui)
set_target_properties(${LIB} PROPERTIES UNITY_BUILD OFF)

if (USE_IMGUITESTENGINE)
	# use ui_test_engine instead of ui to enable imgui test engine
	set(TEST_ENGINE_SRCS
		tests/FileDialogTest.cpp

		dearimgui/imgui_test_engine/imgui_capture_tool.cpp
		dearimgui/imgui_test_engine/imgui_capture_tool.h
		dearimgui/imgui_test_engine/imgui_te_context.cpp
		dearimgui/imgui_test_engine/imgui_te_context.h
		dearimgui/imgui_test_engine/imgui_te_coroutine.cpp
		dearimgui/imgui_test_engine/imgui_te_coroutine.h
		dearimgui/imgui_test_engine/imgui_te_engine.cpp
		dearimgui/imgui_test_engine/imgui_te_engine.h
		dearimgui/imgui_test_engine/imgui_te_exporters.cpp
		dearimgui/imgui_test_engine/imgui_te_exporters.h
		dearimgui/imgui_test_engine/imgui_te_imconfig.h
		dearimgui/imgui_test_engine/imgui_te_internal.h
		dearimgui/imgui_test_engine/imgui_te_perftool.cpp
		dearimgui/imgui_test_engine/imgui_te_perftool.h
		dearimgui/imgui_test_engine/imgui_te_ui.cpp
		dearimgui/imgui_test_engine/imgui_te_ui.h
		dearimgui/imgui_test_engine/imgui_te_utils.cpp
		dearimgui/imgui_test_engine/imgui_te_utils.h
	)

	set(TEST_ENGINE ${LIB}_test_engine)
	engine_add_module(TARGET ${TEST_ENGINE} SRCS ${SRCS} ${TEST_ENGINE_SRCS} DEPENDENCIES render app ${LIBS})

	check_c_compiler_flag("-Wno-shadow" C_NO_SHADOW)
	if (C_NO_SHADOW)
		foreach (src ${TEST_ENGINE_SRCS})
			set_source_files_properties(${src} PROPERTIES COMPILE_FLAGS -Wno-shadow)
		endforeach()
	endif()
	target_compile_definitions(${TEST_ENGINE} PUBLIC ${DEFINITIONS} -DIMGUI_ENABLE_TEST_ENGINE -DIMGUI_TEST_ENGINE_ENABLE_STD_FUNCTION=1 -DIMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL=1 -DIMGUI_TEST_ENGINE_ENABLE_IMPLOT=1)
	target_include_directories(${TEST_ENGINE} PUBLIC . ${GENERATE_DIR}/${LIB} dearimgui dearimgui/imgui_test_engine)
	target_link_libraries(${TEST_ENGINE} PRIVATE ui-fonts)
	set_target_properties(${TEST_ENGINE} PROPERTIES UNITY_BUILD OFF)
endif()
