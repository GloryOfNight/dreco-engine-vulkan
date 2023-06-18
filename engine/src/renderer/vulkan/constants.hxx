#pragma once

namespace de::vulkan::constants
{
	namespace shaders
	{
		inline const char* const basicVert = "basic.vert.spv";
		inline const char* const basicFrag = "basic.frag.spv";

		inline const char* const skyboxVert = "skybox.vert.spv";
		inline const char* const skyboxFrag = "skybox.frag.spv";
	} // namespace shaders

	namespace materials
	{
		inline const char* const basic = "basic";
		inline const char* const skybox = "skybox";
	} // namespace materials
} // namespace de::vulkan::constants