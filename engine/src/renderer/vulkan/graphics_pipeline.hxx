#pragma once

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

		void create(const material& material);

		void recreatePipeline();

		void destroy();

		void bindCmd(vk::CommandBuffer commandBuffer) const;

		vk::PipelineLayout getLayout() const;

		vk::Pipeline get() const;

	protected:
		void createPipelineLayout(vk::Device device);

		void createPipeline(vk::Device device);

	private:
		const material* _owner;

		vk::PipelineLayout _pipelineLayout;

		vk::Pipeline _pipeline;
	};
} // namespace de::vulkan