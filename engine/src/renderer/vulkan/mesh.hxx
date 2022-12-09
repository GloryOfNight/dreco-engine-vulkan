#pragma once
#include "math/mat4.hxx"

#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class mesh final
	{
	public:
		void init(uint32_t vertexCount, size_t vertexSize, uint32_t vertexOffset, uint32_t indexCount, uint32_t indexOffset);

		void drawCmd(vk::CommandBuffer commandBuffer) const;

		// temporal hold of the mesh matrix (transform)
		de::math::mat4 _mat;

		vk::DeviceSize getVertexSize() const;
		vk::DeviceSize getIndexSize() const;

	private:
		uint32_t _vertexOffset{0};
		vk::DeviceSize _vertexSize{0};
		vk::DeviceSize _vertexCount{0};

		uint32_t _indexOffset{0};
		vk::DeviceSize _indexSize{0};
		vk::DeviceSize _indexCount{0};
	};
} // namespace de::vulkan