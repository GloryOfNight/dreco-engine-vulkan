#pragma once

#include <SDL_events.h>
#include <cstdint>
#include <forward_list>
#include <functional>
#include <map>

class event_manager
{
public:
	typedef std::function<void(const SDL_Event&)> event_callback_func;

	struct event_binding_handle
	{
		const uint32_t event;
		const std::forward_list<event_callback_func>::iterator elem;
	};

	event_manager() = default;
	event_manager(const event_manager&) = default;
	event_manager(event_manager&&) = default;
	~event_manager() = default;

	void tick();

	event_binding_handle addEventBinding(uint32_t event, event_callback_func callback);

	void removeEventBinding(const event_binding_handle& handle);

private:
	std::map<uint32_t, std::forward_list<event_callback_func>> eventBindings = {};
};