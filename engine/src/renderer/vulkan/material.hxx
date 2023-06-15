#pragma once

#include "images/texture_image.hxx"

#include "buffer.hxx"
#include "material_instance.hxx"
#include "shader.hxx"

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class material final
	{
		material() = default;

	public:
		using unique = std::unique_ptr<material>;

		static unique makeNew(shader::shared vert, shader::shared frag, size_t maxInstances = 1);
		material_instance* makeInstance();

		material(material&) = delete;
		material(material&&) = default;
		~material();

		void resizeDescriptorPool(uint32_t newSize);

		void bindCmd(vk::CommandBuffer commandBuffer) const;

		const std::vector<vk::DescriptorSetLayout>& getDescriptorSetLayouts() const;
		vk::DescriptorPool getDescriptorPool() const;

		std::vector<vk::PushConstantRange> getPushConstantRanges() const;
		std::vector<vk::PipelineShaderStageCreateInfo> getShaderStages() const;

		const shader::shared& getVertShader() const;
		const shader::shared& getFragShader() const;

		vk::PipelineLayout getPipelineLayout() const;
		vk::Pipeline getPipeline() const;

	private:
		void init(size_t maxInstances);
		void setShaderVert(const shader::shared& inShader);
		void setShaderFrag(const shader::shared& inShader);
		void setInstanceCount(uint32_t inValue);

		void createDescriptorPool(uint32_t maxSets = 1);

		void createPipelineLayout();

		vk::Pipeline createPipeline();

		shader::shared _vert;
		shader::shared _frag;

		std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts;
		vk::DescriptorPool _descriptorPool;

		vk::PipelineLayout _pipelineLayout;
		vk::Pipeline _pipeline;

		std::vector<material_instance::unique> _instances;
	};
} // namespace de::vulkan