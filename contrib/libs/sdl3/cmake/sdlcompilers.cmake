macro(SDL_DetectCompiler)
  set(USE_CLANG FALSE)
  set(USE_GCC FALSE)
  set(USE_INTELCC FALSE)
  set(USE_QCC FALSE)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang|IntelLLVM")
    set(USE_CLANG TRUE)
    # Visual Studio 2019 v16.2 added support for Clang/LLVM.
    # Check if a Visual Studio project is being generated with the Clang toolset.
    if(MSVC)
      set(MSVC_CLANG TRUE)
    endif()
  elseif(CMAKE_COMPILER_IS_GNUCC)
    set(USE_GCC TRUE)
  elseif(CMAKE_C_COMPILER_ID MATCHES "^Intel$")
    set(USE_INTELCC TRUE)
  elseif(CMAKE_C_COMPILER_ID MATCHES "QCC")
    set(USE_QCC TRUE)
  endif()
endmacro()

function(sdl_target_compile_option_all_languages TARGET OPTION)
  target_compile_options(${TARGET} PRIVATE "$<$<COMPILE_LANGUAGE:C,CXX>:${OPTION}>")
  if(CMAKE_OBJC_COMPILER)
    target_compile_options(${TARGET} PRIVATE "$<$<COMPILE_LANGUAGE:OBJC>:${OPTION}>")
  endif()
endfunction()

function(SDL_AddCommonCompilerFlags TARGET)
  option(SDL_WERROR "Enable -Werror" OFF)

  get_property(TARGET_TYPE TARGET "${TARGET}" PROPERTY TYPE)
  if(MSVC)
    cmake_push_check_state()
    check_c_compiler_flag("/W3" COMPILER_SUPPORTS_W3)
    if(COMPILER_SUPPORTS_W3)
      target_compile_options(${TARGET} PRIVATE "/W3")
    endif()
    cmake_pop_check_state()
  endif()

  if(USE_GCC OR USE_CLANG OR USE_INTELCC OR USE_QCC)
    if(MINGW)
      # See if GCC's -gdwarf-4 is supported
      # See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=101377 for why this is needed on Windows
      cmake_push_check_state()
      check_c_compiler_flag("-gdwarf-4" HAVE_GDWARF_4)
      if(HAVE_GDWARF_4)
        target_compile_options(${TARGET} PRIVATE "$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-gdwarf-4>")
      endif()
      cmake_pop_check_state()
    endif()

    # Check for -Wall first, so later things can override pieces of it.
    # Note: clang-cl treats -Wall as -Weverything (which is very loud),
    #       /W3 as -Wall, and /W4 as -Wall -Wextra.  So: /W3 is enough.
    check_c_compiler_flag(-Wall HAVE_GCC_WALL)
    if(MSVC_CLANG)
      target_compile_options(${TARGET} PRIVATE "/W3")
    elseif(HAVE_GCC_WALL)
      sdl_target_compile_option_all_languages(${TARGET} "-Wall")
      if(HAIKU)
        sdl_target_compile_option_all_languages(${TARGET} "-Wno-multichar")
      endif()
    endif()

    check_c_compiler_flag(-Wundef HAVE_GCC_WUNDEF)
    if(HAVE_GCC_WUNDEF)
      sdl_target_compile_option_all_languages(${TARGET} "-Wundef")
    endif()

    check_c_compiler_flag(-Wfloat-conversion HAVE_GCC_WFLOAT_CONVERSION)
    if(HAVE_GCC_WFLOAT_CONVERSION)
      sdl_target_compile_option_all_languages(${TARGET} "-Wfloat-conversion")
    endif()

    check_c_compiler_flag(-fno-strict-aliasing HAVE_GCC_NO_STRICT_ALIASING)
    if(HAVE_GCC_NO_STRICT_ALIASING)
      sdl_target_compile_option_all_languages(${TARGET} "-fno-strict-aliasing")
    endif()

    check_c_compiler_flag(-Wdocumentation HAVE_GCC_WDOCUMENTATION)
    if(HAVE_GCC_WDOCUMENTATION)
      if(SDL_WERROR)
        check_c_compiler_flag(-Werror=documentation HAVE_GCC_WERROR_DOCUMENTATION)
        if(HAVE_GCC_WERROR_DOCUMENTATION)
          sdl_target_compile_option_all_languages(${TARGET} "-Werror=documentation")
        endif()
      endif()
      sdl_target_compile_option_all_languages(${TARGET} "-Wdocumentation")
    endif()

    check_c_compiler_flag(-Wdocumentation-unknown-command HAVE_GCC_WDOCUMENTATION_UNKNOWN_COMMAND)
    if(HAVE_GCC_WDOCUMENTATION_UNKNOWN_COMMAND)
      if(SDL_WERROR)
        check_c_compiler_flag(-Werror=documentation-unknown-command HAVE_GCC_WERROR_DOCUMENTATION_UNKNOWN_COMMAND)
        if(HAVE_GCC_WERROR_DOCUMENTATION_UNKNOWN_COMMAND)
          sdl_target_compile_option_all_languages(${TARGET} "-Werror=documentation-unknown-command")
        endif()
      endif()
      sdl_target_compile_option_all_languages(${TARGET} "-Wdocumentation-unknown-command")
    endif()

    check_c_compiler_flag(-fcomment-block-commands=threadsafety HAVE_GCC_COMMENT_BLOCK_COMMANDS)
    if(HAVE_GCC_COMMENT_BLOCK_COMMANDS)
      sdl_target_compile_option_all_languages(${TARGET} "-fcomment-block-commands=threadsafety")
    else()
      check_c_compiler_flag(/clang:-fcomment-block-commands=threadsafety HAVE_CLANG_COMMENT_BLOCK_COMMANDS)
      if(HAVE_CLANG_COMMENT_BLOCK_COMMANDS)
        sdl_target_compile_option_all_languages(${TARGET} "/clang:-fcomment-block-commands=threadsafety")
      endif()
    endif()

    check_c_compiler_flag(-Wshadow HAVE_GCC_WSHADOW)
    if(HAVE_GCC_WSHADOW)
      sdl_target_compile_option_all_languages(${TARGET} "-Wshadow")
    endif()

    check_c_compiler_flag(-Wunused-local-typedefs HAVE_GCC_WUNUSED_LOCAL_TYPEDEFS)
    if(HAVE_GCC_WUNUSED_LOCAL_TYPEDEFS)
      sdl_target_compile_option_all_languages(${TARGET} "-Wno-unused-local-typedefs")
    endif()

    check_c_compiler_flag(-Wimplicit-fallthrough HAVE_GCC_WIMPLICIT_FALLTHROUGH)
    if(HAVE_GCC_WIMPLICIT_FALLTHROUGH)
      sdl_target_compile_option_all_languages(${TARGET} "-Wimplicit-fallthrough")
    endif()
  endif()

  if(SDL_WERROR)
    if(MSVC)
      check_c_compiler_flag(/WX HAVE_WX)
      if(HAVE_WX)
        target_compile_options(${TARGET} PRIVATE "/WX")
      endif()
    elseif(USE_GCC OR USE_CLANG OR USE_INTELCC OR USE_QNX)
      check_c_compiler_flag(-Werror HAVE_WERROR)
      if(HAVE_WERROR)
        sdl_target_compile_option_all_languages(${TARGET} "-Werror")
      endif()

      if(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
        check_linker_flag(C "-Wl,--no-undefined-version" LINKER_SUPPORTS_NO_UNDEFINED_VERSION)
        if(LINKER_SUPPORTS_NO_UNDEFINED_VERSION)
          target_link_options(${TARGET} PRIVATE "-Wl,--no-undefined-version")
        endif()
      endif()
    endif()
  endif()

  if(USE_CLANG)
    check_c_compiler_flag("-fcolor-diagnostics" COMPILER_SUPPORTS_FCOLOR_DIAGNOSTICS)
    if(COMPILER_SUPPORTS_FCOLOR_DIAGNOSTICS)
      sdl_target_compile_option_all_languages(${TARGET} "-fcolor-diagnostics")
    endif()
  else()
    check_c_compiler_flag("-fdiagnostics-color=always" COMPILER_SUPPORTS_FDIAGNOSTICS_COLOR_ALWAYS)
    if(COMPILER_SUPPORTS_FDIAGNOSTICS_COLOR_ALWAYS)
      sdl_target_compile_option_all_languages(${TARGET} "-fdiagnostics-color=always")
    endif()
  endif()
endfunction()
