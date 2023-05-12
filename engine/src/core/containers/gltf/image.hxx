#pragma once
#include "core/containers/image_data.hxx"

#include <string>

namespace de::gltf
{
	struct image
	{
		std::string _uri;
		image_data _image;
	};
} // namespace de::gltf
