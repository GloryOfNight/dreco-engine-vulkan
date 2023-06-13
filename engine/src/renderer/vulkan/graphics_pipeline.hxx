#pragma once

#include "shader.hxx"

#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class material;
	class graphics_pipeline final
	{
	public:
		graphics_pipeline() = default;
		graphics_pipeline(const graphics_pipeline&) = delete;
		graphics_pipeline(graphics_pipeline&&) = default;
		~graphics_pipeline() { destroy(); };

		void create(vk::PipelineLayout pipelineLayout, shader::shared vertShader, shader::shared fragShader);

		void destroy();

		void bindCmd(vk::CommandBuffer commandBuffer) const;

		vk::Pipeline get() const;

	protected:
		void createPipeline(vk::Device device);

	private:
		vk::Pipeline _pipeline;
	};
} // namespace de::vulkan