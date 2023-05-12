#include "input_manager.hxx"

#include "platforms/platform.h"
#include "renderer/render.hxx"

#include <SDL_events.h>

de::input_manager::input_manager(event_manager& _eventManager)
{
	_eventManager.addEventBindingMem(SDL_EVENT_KEY_DOWN, this, &input_manager::onKeyEvent);
	_eventManager.addEventBindingMem(SDL_EVENT_KEY_UP, this, &input_manager::onKeyEvent);
	_eventManager.addEventBindingMem(SDL_EVENT_MOUSE_MOTION, this, &input_manager::onMouseEvent);
	_eventManager.addEventBindingMem(SDL_EVENT_MOUSE_BUTTON_DOWN, this, &input_manager::onMouseEvent);
	_eventManager.addEventBindingMem(SDL_EVENT_MOUSE_BUTTON_UP, this, &input_manager::onMouseEvent);
	_eventManager.addEventBindingMem(SDL_EVENT_MOUSE_WHEEL, this, &input_manager::onMouseEvent);
}

bool de::input_manager::isKeyPressed(const uint32_t key) const
{
	const auto& findRes = _keys.find(key);
	if (findRes != _keys.end() && findRes->second._state == SDL_PRESSED)
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
	SDL_WarpMouseInWindow(de::renderer::get()->getWindow(), x, y);
	_mouseState._x = x;
	_mouseState._y = y;
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
	// SDL Relative mode acting wierd on linux, untill figure out, use hack:
#if PLATFORM_LINUX
	showCursor(!state);
#else
	SDL_SetRelativeMouseMode(static_cast<SDL_bool>(state));
#endif
}

bool de::input_manager::isInMouseFocus() const
{
	return _inMouseFocus;
}

void de::input_manager::onKeyEvent(const SDL_Event& event)
{
	_keys.insert_or_assign(event.key.keysym.sym, key_state(event));
}

void de::input_manager::onMouseEvent(const SDL_Event& event)
{
	const uint32_t windowId = de::vulkan::renderer::get()->getWindowId();
	_inMouseFocus = event.motion.windowID == windowId ||
					event.button.windowID == windowId ||
					event.wheel.windowID == windowId;

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
			_mouseState._state = event.button.state;
			_mouseState._x = event.button.x;
			_mouseState._y = event.button.y;
		}
	}
}
