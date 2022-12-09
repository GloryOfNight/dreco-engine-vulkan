#pragma once

#include <stdexcept>

namespace de::except
{
	class initialization_error : public std::runtime_error
	{
	public:
		initialization_error()
			: std::runtime_error("Initialization Error")
		{
		}
	};
	
	class out_of_space : public std::length_error
	{
	public:
		out_of_space()
			: std::length_error("Out of Space")
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
} // namespace de::except