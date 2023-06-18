#pragma once

#include "images/cubemap_image.hxx"

#include "buffer.hxx"
#include "material_instance.hxx"

#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class skybox
	{
	public:
		void init();

		void destroy();

		void drawCmd(vk::CommandBuffer commandBuffer);

	private:
		void createBox();

		cubemap_image _cubemap;
		class material_instance* _matInst;

		buffer::id _boxMeshId;

		vk::DeviceSize _vertSize{};
	};
} // namespace de::vulkan