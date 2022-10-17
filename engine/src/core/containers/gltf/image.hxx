#pragma once
#include <string>
#include "core/containers/image_data.hxx"

namespace gltf
{
	struct image
	{
		std::string _uri;
		image_data _image;
	};
} // namespace gltf
