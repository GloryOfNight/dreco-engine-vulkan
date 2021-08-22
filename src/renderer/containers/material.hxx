#pragma once

#include "texture_data.hxx"

struct material
{
	material()
		: _texData{nullptr} {};

	texture_data* _texData;
};