#include "input_manager.hxx"

#include <SDL2/SDL_events.h>

input_manager::input_manager(event_manager& _eventManager)
{
	_eventManager.addEventBindingMem(SDL_KEYDOWN, this, &input_manager::onKeyEvent);
	_eventManager.addEventBindingMem(SDL_KEYUP, this, &input_manager::onKeyEvent);
}

bool input_manager::isKeyPressed(const uint32_t key) const
{
	return _pressedKeys.find(key) != _pressedKeys.end();
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