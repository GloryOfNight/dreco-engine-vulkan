#pragma once

#include "dreco.hxx"
#include "event_manager.hxx"

#include <set>

namespace de
{
	class DRECO_API input_manager
	{
	private:
		struct mouse_state
		{
			uint32_t _state{};
			uint16_t _x{};
			uint16_t _y{};
		};

	public:
		input_manager(event_manager& _eventManager);
		input_manager(const input_manager&) = delete;
		input_manager(input_manager&&) = default;
		~input_manager() = default;

		bool isKeyPressed(const uint32_t key) const;

		uint32_t getMouseState(uint16_t* const x, uint16_t* const y) const;

		void warpMouse(const uint16_t x, const uint16_t y);

		void showCursor(const bool state) const;

		void setMouseRelativeMode(const bool state) const;

		bool isInMouseFocus() const;

	protected:
		void onKeyEvent(const SDL_Event& event);

		void onMouseEvent(const SDL_Event& event);

	private:
		std::set<uint32_t> _pressedKeys;

		mouse_state _mouseState;

		bool _inMouseFocus{};
	};
} // namespace de