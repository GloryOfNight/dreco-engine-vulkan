#include "event_manager.hxx"

#include "core/utils/log.hxx"

void event_manager::tick()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (const auto bindIt = eventBindings.find(event.type); bindIt != eventBindings.end())
		{
			for (auto bindFunc : bindIt->second)
			{
				try
				{
					std::invoke(bindFunc, event);
				}
				catch (std::bad_function_call badFuncCall)
				{
					DE_LOG(Error, "%s: BadFunctionCall: %s", __FUNCTION__, badFuncCall.what());
				}
			}
		}
	}
}

event_manager::event_binding_handle event_manager::addEventBinding(uint32_t event, event_callback_func callback)
{
	auto binding = eventBindings.try_emplace(event);
	binding.first->second.push_front(callback);

	return {event, binding.first->second.before_begin()};
}

void event_manager::removeEventBinding(const event_binding_handle& handle)
{
	if (const auto bindIt = eventBindings.find(handle.event); bindIt != eventBindings.end())
	{
		bindIt->second.erase_after(handle.elem);
	}
}