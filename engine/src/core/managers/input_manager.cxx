#include "input_manager.hxx"

#include "core/engine.hxx"
#include "platforms/platform.h"

#include <SDL3/SDL_events.h>

void de::input_manager::init(event_manager& eventManager)
{
	eventManager.addEventBindingMem(SDL_EVENT_KEY_DOWN, this, &input_manager::onKeyEvent);
	eventManager.addEventBindingMem(SDL_EVENT_KEY_UP, this, &input_manager::onKeyEvent);
	eventManager.addEventBindingMem(SDL_EVENT_MOUSE_MOTION, this, &input_manager::onMouseEvent);
	eventManager.addEventBindingMem(SDL_EVENT_MOUSE_BUTTON_DOWN, this, &input_manager::onMouseEvent);
	eventManager.addEventBindingMem(SDL_EVENT_MOUSE_BUTTON_UP, this, &input_manager::onMouseEvent);
	eventManager.addEventBindingMem(SDL_EVENT_MOUSE_WHEEL, this, &input_manager::onMouseEvent);
}

bool de::input_manager::isKeyPressed(const uint32_t key) const
{
	const auto& findRes = _keys.find(key);
	if (findRes != _keys.end() && findRes->second._state == true)
	{
		return true;
	}
	return false;
}

uint32_t de::input_manager::getMouseState(uint16_t* const x, uint16_t* const y) const
{
	if (x)
		*x = _mouseState._x;
	if (y)
		*y = _mouseState._y;
	return _mouseState._state;
}

void de::input_manager::warpMouse(const uint16_t x, const uint16_t y)
{
	auto window = SDL_GetWindowFromID(_windowId);
	if (window)
	{
		SDL_WarpMouseInWindow(window, x, y);
		_mouseState._x = x;
		_mouseState._y = y;
	}
}

void de::input_manager::showCursor(const bool state) const
{
	if (state)
	{
		SDL_ShowCursor();
	}
	else
	{
		SDL_HideCursor();
	}
}

void de::input_manager::setMouseRelativeMode(const bool state) const
{
	// SDL Relative mode acting weird on Linux, until figure out, use hack:
#if PLATFORM_LINUX
	showCursor(!state);
#else
	// SDL_SetRelativeMouseMode(static_cast<SDL_bool>(state)); ???
#endif
}

bool de::input_manager::isInMouseFocus() const
{
	return _inMouseFocus;
}

void de::input_manager::onKeyEvent(const SDL_Event& event)
{
	// ignore input that was for wrong window
	if (event.key.windowID != _windowId && _windowId != UINT32_MAX)
	{
		return;
	}

	_keys.insert_or_assign(event.key.key, key_state(event));
}

void de::input_manager::onMouseEvent(const SDL_Event& event)
{
	// ignore input that was for wrong window
	if (event.motion.windowID != _windowId && _windowId != UINT32_MAX)
	{
		return;
	}

	_inMouseFocus = true;

	if (_inMouseFocus)
	{
		if (event.type == SDL_EVENT_MOUSE_MOTION)
		{
			_mouseState._state = event.motion.state;
			_mouseState._x = event.motion.x;
			_mouseState._y = event.motion.y;
		}
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP)
		{
			_mouseState._state = event.button.down;
			_mouseState._x = event.button.x;
			_mouseState._y = event.button.y;
		}
	}
}

void de::input_manager::setWindowId(uint32_t windowId)
{
	_windowId = windowId;
}
