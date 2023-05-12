#pragma once
#include <vector>

namespace de::gltf
{
	struct scene
	{
		std::string _name;
		std::vector<uint32_t> _nodes;
	};
} // namespace de::gltf