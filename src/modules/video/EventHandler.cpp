/**
 * @file
 */

#include "EventHandler.h"
#include "video/IEventObserver.h"
#include "core/GLM.h"
#include "core/Assert.h"
#include <SDL3/SDL_events.h>
#include <glm/common.hpp>
#include <SDL3/SDL.h>

namespace video {

EventHandler::EventHandler() :
		_multiGesture(false) {
	SDL_SetEventEnabled(SDL_EVENT_JOYSTICK_AXIS_MOTION, true);
	SDL_SetEventEnabled(SDL_EVENT_DROP_FILE, true);
	SDL_SetEventEnabled(SDL_EVENT_DROP_TEXT, true);
}

EventHandler::~EventHandler() {
}

void EventHandler::registerObserver(IEventObserver* observer) {
	core_assert(observer != nullptr);
	_events.emplace_back(observer, false);
}

void EventHandler::removeObserver(IEventObserver* observer) {
	core_assert(observer != nullptr);
	_events.emplace_back(observer, true);
}

core::String EventHandler::getControllerButtonName(uint8_t button) {
	const char *name = SDL_GetGamepadStringForButton(static_cast<SDL_GamepadButton>(button));
	if (name == nullptr) {
		return "unknown";
	}
	return name;
}

bool EventHandler::handleEvent(SDL_Event &event) {
	switch (event.type) {
	case SDL_EVENT_DROP_FILE:
		dropFile((void*)SDL_GetWindowFromID(event.drop.windowID), event.drop.data);
		break;
	case SDL_EVENT_DROP_TEXT:
		dropText((void*)SDL_GetWindowFromID(event.drop.windowID), event.drop.data);
		break;
	case SDL_EVENT_TEXT_INPUT:
		textInput((void*)SDL_GetWindowFromID(event.text.windowID), core::String(event.text.text));
		break;
	case SDL_EVENT_KEY_UP:
		keyRelease((void*)SDL_GetWindowFromID(event.key.windowID), (int32_t) event.key.key, (int16_t) event.key.mod);
		break;
	case SDL_EVENT_KEY_DOWN:
		keyPress((void*)SDL_GetWindowFromID(event.key.windowID), (int32_t) event.key.key, (int16_t) event.key.mod);
		break;
	case SDL_EVENT_MOUSE_MOTION: {
		if (event.motion.which == SDL_TOUCH_MOUSEID) {
			break;
		}
		SDL_Window *window = SDL_GetWindowFromID(event.motion.windowID);
		mouseMotion((void*)window, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel, event.motion.which);
		break;
	}
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		if (event.button.which == SDL_TOUCH_MOUSEID) {
			break;
		}
		mouseButtonPress((void*)SDL_GetWindowFromID(event.button.windowID), event.button.x, event.button.y, event.button.button, event.button.clicks, event.button.which);
		break;
	case SDL_EVENT_MOUSE_BUTTON_UP:
		if (event.button.which == SDL_TOUCH_MOUSEID) {
			break;
		}
		mouseButtonRelease((void*)SDL_GetWindowFromID(event.button.windowID), event.button.x, event.button.y, event.button.button, event.button.which);
		break;
	case SDL_EVENT_MOUSE_WHEEL: {
		if (event.wheel.which == SDL_TOUCH_MOUSEID) {
			break;
		}
		float x = event.wheel.x;
		float y = event.wheel.y;
		x = glm::clamp(x, -1.0f, 1.0f);
		y = glm::clamp(y, -1.0f, 1.0f);
		mouseWheel((void*)SDL_GetWindowFromID(event.wheel.windowID), x, y, event.wheel.which);
		break;
	}
	case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		controllerMotion(event.gaxis.axis, event.gaxis.value, event.gaxis.which);
		break;
	case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		controllerButtonPress(getControllerButtonName(event.gbutton.button), event.gbutton.which);
		break;
	case SDL_EVENT_GAMEPAD_BUTTON_UP:
		controllerButtonRelease(getControllerButtonName(event.gbutton.button), event.gbutton.which);
		break;
	case SDL_EVENT_GAMEPAD_ADDED:
		controllerDeviceAdded(event.cdevice.which);
		break;
	case SDL_EVENT_GAMEPAD_REMOVED:
		controllerDeviceRemoved(event.cdevice.which);
		break;
	case SDL_EVENT_JOYSTICK_HAT_MOTION:
	case SDL_EVENT_JOYSTICK_ADDED:
	case SDL_EVENT_JOYSTICK_REMOVED:
	case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
	case SDL_EVENT_JOYSTICK_BUTTON_UP:
	case SDL_EVENT_JOYSTICK_AXIS_MOTION:
	case SDL_EVENT_JOYSTICK_BALL_MOTION:
		// ignore joystick events - use gamecontroller events
		break;
	case SDL_EVENT_FINGER_DOWN:
		fingerPress((void*)SDL_GetWindowFromID(event.key.windowID), event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
		break;
	case SDL_EVENT_FINGER_UP:
		fingerRelease((void*)SDL_GetWindowFromID(event.key.windowID), event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
		break;
	case SDL_EVENT_FINGER_MOTION:
		fingerMotion((void*)SDL_GetWindowFromID(event.key.windowID), event.tfinger.fingerID, event.tfinger.x, event.tfinger.y, event.tfinger.dx, event.tfinger.dy);
		break;
	case SDL_EVENT_WINDOW_RESTORED: {
		SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);
		for (IEventObserver* observer : _observers) {
			observer->onWindowRestore((void*)window);
		}
		break;
	}
	case SDL_EVENT_WINDOW_RESIZED: {
		SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);
		for (IEventObserver* observer : _observers) {
			const int w = event.window.data1;
			const int h = event.window.data2;
			observer->onWindowResize((void*)window, w, h);
		}
		break;
	}
	case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
		SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);
		for (IEventObserver* observer : _observers) {
			observer->onWindowClose((void*)window);
		}
		break;
	}
	case SDL_EVENT_WINDOW_MOVED: {
		SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);
		for (IEventObserver* observer : _observers) {
			observer->onWindowMoved((void*)window);
		}
		break;
	}
	case SDL_EVENT_WINDOW_FOCUS_GAINED: {
		SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);
		for (IEventObserver* observer : _observers) {
			observer->onWindowFocusGained((void*)window);
		}
		break;
	}
	case SDL_EVENT_WINDOW_FOCUS_LOST: {
		SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);
		for (IEventObserver* observer : _observers) {
			observer->onWindowFocusLost((void*)window);
		}
		break;
	}
	default:
		break;
	}

	// Traverse through the list and try to find the specified observer
	for (const Event& obsEvent : _events) {
		if (obsEvent.remove) {
			for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
				if (*i == obsEvent.observer) {
					_observers.erase(i);
					break;
				}
			}
		} else {
			_observers.push_back(obsEvent.observer);
		}
	}
	_events.clear();

	return true;
}

bool EventHandler::handleAppEvent(SDL_Event &event) {
	switch (event.type) {
	case SDL_EVENT_TERMINATING:
		prepareShutdown();
		break;
	case SDL_EVENT_LOW_MEMORY:
		lowMemory();
		break;
	case SDL_EVENT_WILL_ENTER_BACKGROUND:
		prepareBackground();
		return true;
	case SDL_EVENT_DID_ENTER_BACKGROUND:
		background();
		return true;
	case SDL_EVENT_WILL_ENTER_FOREGROUND:
		prepareForeground();
		return true;
	case SDL_EVENT_DID_ENTER_FOREGROUND:
		foreground();
		return true;
	}
	return false;
}

void EventHandler::lowMemory() {
	for (IEventObserver* observer : _observers) {
		observer->onLowMemory();
	}
}

void EventHandler::prepareShutdown() {
	for (IEventObserver* observer : _observers) {
		observer->onPrepareShutdown();
	}
}

void EventHandler::prepareBackground() {
	for (IEventObserver* observer : _observers) {
		observer->onPrepareBackground();
	}
}

void EventHandler::prepareForeground() {
	for (IEventObserver* observer : _observers) {
		observer->onPrepareForeground();
	}
}

void EventHandler::background() {
	for (IEventObserver* observer : _observers) {
		observer->onBackground();
	}
}

void EventHandler::foreground() {
	for (IEventObserver* observer : _observers) {
		observer->onForeground();
	}
}

void EventHandler::mouseWheel(void *windowHandle, float x, float y, int32_t mouseId) {
	for (IEventObserver* observer : _observers) {
		observer->onMouseWheel(windowHandle, x, y, mouseId);
	}
}

void EventHandler::mouseMotion(void *windowHandle, int32_t x, int32_t y, int32_t relX, int32_t relY, int32_t mouseId) {
	for (IEventObserver* observer : _observers) {
		observer->onMouseMotion(windowHandle, x, y, relX, relY, mouseId);
	}
}

void EventHandler::controllerDeviceAdded(int32_t device) {
	for (IEventObserver* observer : _observers) {
		observer->onControllerDeviceAdded(device);
	}
}

void EventHandler::controllerDeviceRemoved(int32_t device) {
	for (IEventObserver* observer : _observers) {
		observer->onControllerDeviceRemoved(device);
	}
}

void EventHandler::controllerMotion(uint8_t axis, int value, uint32_t id) {
	for (IEventObserver* observer : _observers) {
		observer->onControllerMotion(axis, value, id);
	}
}

void EventHandler::controllerButtonPress(const core::String& button, uint32_t id) {
	for (IEventObserver* observer : _observers) {
		observer->onControllerButtonPress(button, id);
	}
}

void EventHandler::controllerButtonRelease(const core::String& button, uint32_t id) {
	for (IEventObserver* observer : _observers) {
		observer->onControllerButtonRelease(button, id);
	}
}

void EventHandler::mouseButtonPress(void *windowHandle, int32_t x, int32_t y, uint8_t button, uint8_t clicks, int32_t mouseId) {
	for (IEventObserver* observer : _observers) {
		observer->onMouseButtonPress(windowHandle, x, y, button, clicks, mouseId);
	}
}

void EventHandler::mouseButtonRelease(void *windowHandle, int32_t x, int32_t y, uint8_t button, int32_t mouseId) {
	for (IEventObserver* observer : _observers) {
		observer->onMouseButtonRelease(windowHandle, x, y, button, mouseId);
	}
}

void EventHandler::dropFile(void *windowHandle, const core::String& file) {
	for (IEventObserver* observer : _observers) {
		observer->onDropFile(windowHandle, file);
	}
}

void EventHandler::dropText(void *windowHandle, const core::String& text) {
	for (IEventObserver* observer : _observers) {
		observer->onDropText(windowHandle, text);
	}
}

void EventHandler::textInput(void *windowHandle, const core::String& text) {
	for (IEventObserver* observer : _observers) {
		observer->onTextInput(windowHandle, text);
	}
}

void EventHandler::keyRelease(void *windowHandle, int32_t key, int16_t modifier) {
	for (IEventObserver* observer : _observers) {
		observer->onKeyRelease(windowHandle, key, modifier);
	}
}

void EventHandler::keyPress(void *windowHandle, int32_t key, int16_t modifier) {
	for (IEventObserver* observer : _observers) {
		observer->onKeyPress(windowHandle, key, modifier);
	}
}

void EventHandler::fingerPress(void *windowHandle, int64_t finger, float x, float y) {
	for (IEventObserver* observer : _observers) {
		observer->onFingerPress(windowHandle, finger, x, y);
	}
}

void EventHandler::fingerRelease(void *windowHandle, int64_t finger, float x, float y) {
	_multiGesture = false;
	for (IEventObserver* observer : _observers) {
		observer->onFingerRelease(windowHandle, finger, x, y);
	}
}

void EventHandler::fingerMotion(void *windowHandle, int64_t finger, float x, float y, float dx, float dy) {
	if (_multiGesture) {
		return;
	}
	for (IEventObserver* observer : _observers) {
		observer->onFingerMotion(windowHandle, finger, x, y, dx, dy);
	}
}

}
