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
		const uint32_t event{};
		const std::forward_list<event_callback_func>::iterator elem{};
	};

	event_manager() = default;
	event_manager(const event_manager&) = default;
	event_manager(event_manager&&) = default;
	~event_manager() = default;

	void tick();

	template<typename T, typename F>
	event_binding_handle addEventBindingMem(const uint32_t event, T* obj, F func);

	event_binding_handle addEventBinding(uint32_t event, event_callback_func callback);

	void removeEventBinding(const event_binding_handle& handle);

private:
	std::map<uint32_t, std::forward_list<event_callback_func>> eventBindings{};
};

template <typename T, typename F>
inline event_manager::event_binding_handle event_manager::addEventBindingMem(const uint32_t event, T* obj, F func)
{
	return addEventBinding(event, std::bind(func, obj, std::placeholders::_1));
}
