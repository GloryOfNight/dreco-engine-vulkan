#pragma once

#include "dreco.hxx"

#include <string>

namespace de::file
{
	DRECO_API std::string read(const std::string_view path);
}