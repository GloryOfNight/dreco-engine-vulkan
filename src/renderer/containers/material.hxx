#pragma once

#include "texture_data.hxx"

struct material_data
{
	material_data()
		: _texData{nullptr} {};

	texture_data* _texData;
};