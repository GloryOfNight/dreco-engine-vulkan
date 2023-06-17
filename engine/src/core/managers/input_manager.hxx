#pragma once

#include "dreco.hxx"
#include "event_manager.hxx"

#include <map>

namespace de
{
	class DRECO_API input_manager
	{
	public:
		struct mouse_state
		{
			uint32_t _state{};
			uint16_t _x{};
			uint16_t _y{};
		};

		struct key_state
		{
			key_state() = default;
			key_state(const SDL_Event& event)
				: _state{event.key.state}
				, _timestamp{event.key.timestamp}
				, _repeat{static_cast<bool>(event.key.repeat)}
			{
			}

			uint32_t _state;
			uint64_t _timestamp;
			bool _repeat;
		};

		input_manager() = default;
		input_manager(const input_manager&) = delete;
		input_manager(input_manager&&) = default;
		~input_manager() = default;

		void init(event_manager& eventManager);

		bool isKeyPressed(const uint32_t key) const;

		uint32_t getMouseState(uint16_t* const x, uint16_t* const y) const;

		void warpMouse(const uint16_t x, const uint16_t y);

		void showCursor(const bool state) const;

		void setMouseRelativeMode(const bool state) const;

		bool isInMouseFocus() const;

		void setWindowId(const uint32_t windowId);

	protected:
		void onKeyEvent(const SDL_Event& event);

		void onMouseEvent(const SDL_Event& event);

	private:
		std::map<uint32_t, key_state> _keys{};

		mouse_state _mouseState{};

		bool _inMouseFocus{};

		uint32_t _windowId{UINT32_MAX};
	};
} // namespace de