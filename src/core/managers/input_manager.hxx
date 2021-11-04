#pragma once

#include <set>

#include "event_manager.hxx"

class input_manager
{
public:
	input_manager(event_manager& _eventManager);
	input_manager(const input_manager&) = delete;
	input_manager(input_manager&&) = delete;
	~input_manager() = default;

	bool isKeyPressed(const uint32_t key) const;

protected:
	void onKeyEvent(const SDL_Event& event);

private:
	std::set<uint32_t> _pressedKeys;
};