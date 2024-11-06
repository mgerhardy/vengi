/**
 * @file
 */

#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>
#include "core/String.h"

#define CUSTOM_SDL_KEYCODE(X) SDL_SCANCODE_TO_KEYCODE((util::button::CUSTOM_SCANCODES + (X)))

namespace util {
namespace button {

#ifdef SDL_PLATFORM_APPLE
#define SDL_KMOD_CONTROL SDL_KMOD_GUI
#define SDL_KMOD_LCONTROL SDL_KMOD_LGUI
#define SDL_KMOD_RCONTROL SDL_KMOD_RGUI
#else
#define SDL_KMOD_CONTROL SDL_KMOD_CTRL
#define SDL_KMOD_LCONTROL SDL_KMOD_LCTRL
#define SDL_KMOD_RCONTROL SDL_KMOD_RCTRL
#endif

#define CUSTOM_SDL_BUTTON_OFFSET (SDL_BUTTON_X2 + 10)
static const int32_t CUSTOM_SCANCODES               = SDL_SCANCODE_COUNT + 1;
static const int32_t CUSTOM_SDLK_MOUSE_LEFT         = CUSTOM_SDL_KEYCODE(SDL_BUTTON_LEFT);
static const int32_t CUSTOM_SDLK_MOUSE_MIDDLE       = CUSTOM_SDL_KEYCODE(SDL_BUTTON_MIDDLE);
static const int32_t CUSTOM_SDLK_MOUSE_RIGHT        = CUSTOM_SDL_KEYCODE(SDL_BUTTON_RIGHT);
static const int32_t CUSTOM_SDLK_MOUSE_X1           = CUSTOM_SDL_KEYCODE(SDL_BUTTON_X1);
static const int32_t CUSTOM_SDLK_MOUSE_X2           = CUSTOM_SDL_KEYCODE(SDL_BUTTON_X2);
static const int32_t CUSTOM_SDLK_MOUSE_WHEEL_UP     = CUSTOM_SDL_KEYCODE(CUSTOM_SDL_BUTTON_OFFSET + 0);
static const int32_t CUSTOM_SDLK_MOUSE_WHEEL_DOWN   = CUSTOM_SDL_KEYCODE(CUSTOM_SDL_BUTTON_OFFSET + 1);
static const int32_t CUSTOM_SDLK_MOUSE_WHEEL_LEFT   = CUSTOM_SDL_KEYCODE(CUSTOM_SDL_BUTTON_OFFSET + 2);
static const int32_t CUSTOM_SDLK_MOUSE_WHEEL_RIGHT  = CUSTOM_SDL_KEYCODE(CUSTOM_SDL_BUTTON_OFFSET + 3);
#undef CUSTOM_SDL_BUTTON_OFFSET

static const struct CustomButtonMapping {
	int32_t key;
	const char *name;
	uint16_t count;
} CUSTOMBUTTONMAPPING[] = {
	{CUSTOM_SDLK_MOUSE_LEFT, "left_mouse", 1u},
	{CUSTOM_SDLK_MOUSE_MIDDLE, "middle_mouse", 1u},
	{CUSTOM_SDLK_MOUSE_RIGHT, "right_mouse", 1u},
	{CUSTOM_SDLK_MOUSE_LEFT, "double_left_mouse", 2u},
	{CUSTOM_SDLK_MOUSE_MIDDLE, "double_middle_mouse", 2u},
	{CUSTOM_SDLK_MOUSE_RIGHT, "double_right_mouse", 2u},
	{CUSTOM_SDLK_MOUSE_X1, "x1_mouse", 1u},
	{CUSTOM_SDLK_MOUSE_X2, "x2_mouse", 1u},
	{CUSTOM_SDLK_MOUSE_WHEEL_UP, "wheelup", 1u},
	{CUSTOM_SDLK_MOUSE_WHEEL_DOWN, "wheeldown", 1u},
	{CUSTOM_SDLK_MOUSE_WHEEL_LEFT, "wheelleft", 1u},
	{CUSTOM_SDLK_MOUSE_WHEEL_RIGHT, "wheelright", 1u},
};

// note: doesn't contain all combinations
static constexpr struct ModifierMapping {
	int16_t modifier;
	const char *name;
} MODIFIERMAPPING[] = {
	{SDL_KMOD_LSHIFT, "left_shift"},
	{SDL_KMOD_RSHIFT, "right_shift"},
	{SDL_KMOD_LCONTROL, "left_ctrl"},
	{SDL_KMOD_RCONTROL, "right_ctrl"},
	{SDL_KMOD_LALT, "left_alt"},
	{SDL_KMOD_RALT, "right_alt"},
	{SDL_KMOD_ALT, "alt"},
	{SDL_KMOD_SHIFT, "shift"},
	{SDL_KMOD_CONTROL, "ctrl"},
	{SDL_KMOD_ALT | SDL_KMOD_SHIFT, "alt+shift"},
	{SDL_KMOD_CONTROL | SDL_KMOD_SHIFT, "ctrl+shift"},
	{SDL_KMOD_ALT | SDL_KMOD_CONTROL, "alt+ctrl"},
	{SDL_KMOD_CONTROL | SDL_KMOD_ALT | SDL_KMOD_SHIFT, "ctrl+alt+shift"},
	{0, nullptr}
};

}
}
