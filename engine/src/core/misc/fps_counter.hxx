#pragma once

#include "dreco.hxx"

namespace de::misc
{
	struct fps_counter
	{
		fps_counter() = default;
		void tick(double deltaTime)
		{
			_time -= deltaTime;
			if (_time <= 0.F)
			{
				DE_LOG(Info, "%s: %i", __FUNCTION__, _counter);
				_time = _resetTime - _time;
				_counter = 0;
			}
			++_counter;
		}

	private:
		const double _resetTime{1.f};
		double _time{_resetTime};
		uint64_t _counter{};
	};
} // namespace de::misc
