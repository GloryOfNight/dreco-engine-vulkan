#pragma once

#include <stdexcept>
#include <string_view>

namespace de::vulkan::exept
{
	class out_of_space : public std::runtime_error
	{
	public:
		out_of_space()
			: std::runtime_error("Out of Space")
		{
		}
	};

	class no_gpu : public std::runtime_error
	{
	public:
		no_gpu()
			: std::runtime_error("No Vulkan supported GPU")
		{
		}
	};

	class out_of_range : public std::out_of_range
	{
	public:
		out_of_range()
			: std::out_of_range("Out of Range")
		{
		}
	};
} // namespace de::vulkan::exept