#pragma once

#include <stdexcept>

namespace de::vulkan::exept
{
	class no_gpu : public std::runtime_error
	{
	public:
		no_gpu()
			: std::runtime_error("No Vulkan supported GPU")
		{
		}
	};
} // namespace de::vulkan::exept