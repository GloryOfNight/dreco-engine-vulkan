#include "input_manager.hxx"

#include "renderer/vulkan/vk_renderer.hxx"

#include <SDL2/SDL_events.h>

input_manager::input_manager(event_manager& _eventManager)
{
	_eventManager.addEventBindingMem(SDL_KEYDOWN, this, &input_manager::onKeyEvent);
	_eventManager.addEventBindingMem(SDL_KEYUP, this, &input_manager::onKeyEvent);
	_eventManager.addEventBindingMem(SDL_MOUSEMOTION, this, &input_manager::onMouseEvent);
	_eventManager.addEventBindingMem(SDL_MOUSEBUTTONDOWN, this, &input_manager::onMouseEvent);
	_eventManager.addEventBindingMem(SDL_MOUSEBUTTONUP, this, &input_manager::onMouseEvent);
	_eventManager.addEventBindingMem(SDL_MOUSEWHEEL, this, &input_manager::onMouseEvent);
}

bool input_manager::isKeyPressed(const uint32_t key) const
{
	return _pressedKeys.find(key) != _pressedKeys.end();
}

uint32_t input_manager::getMouseState(uint16_t* const x, uint16_t* const y) const
{
	if (x)
		*x = _mouseState._x;
	if (y)
		*y = _mouseState._y;
	return _mouseState._state;
}

void input_manager::warpMouse(const uint16_t x, const uint16_t y)
{
	if (_mouseState._x != x || _mouseState._y != y)
	{
		SDL_WarpMouseInWindow(vk_renderer::get()->getWindow(), x, y);
		_mouseState._x = x;
		_mouseState._y = y;
	}
}

void input_manager::showCursor(const bool state) const
{
	SDL_ShowCursor(state ? SDL_ENABLE : SDL_DISABLE);
}

bool input_manager::isInMouseFocus() const
{
	return _inMouseFocus;
}

void input_manager::onKeyEvent(const SDL_Event& event)
{
	if (event.key.state == SDL_PRESSED)
	{
		if (!event.key.repeat)
		{
			_pressedKeys.emplace(event.key.keysym.sym);
		}
	}
	else
	{
		_pressedKeys.erase(event.key.keysym.sym);
	}
}

void input_manager::onMouseEvent(const SDL_Event& event)
{
	const uint32_t windowId = vk_renderer::get()->getWindowId();
	_inMouseFocus = event.motion.windowID == windowId ||
					event.button.windowID == windowId ||
					event.wheel.windowID == windowId;

	if (_inMouseFocus)
	{
		if (event.type == SDL_MOUSEMOTION)
		{
			_mouseState._state = event.motion.state;
			_mouseState._x = event.motion.x;
			_mouseState._y = event.motion.y;
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
		{
			_mouseState._state = event.button.state;
			_mouseState._x = event.button.x;
			_mouseState._y = event.button.y;
		}
	}
}
