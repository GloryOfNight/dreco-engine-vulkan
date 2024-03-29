#pragma once

#include "shader.hxx"

#include <memory>
#include <spirv-reflect/spirv_reflect.h>
#include <string>
#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class shader final
	{
	public:
		struct descripted_data
		{
			uint32_t _descriptorSetIndex{UINT32_MAX};

			vk::DescriptorSetLayoutCreateInfo _descriptorSetLayoutCreateInfo{};

			std::vector<vk::DescriptorSetLayoutBinding> _descriptorSetLayoutBindings{};

			std::vector<vk::DescriptorPoolSize> getDescriptorPoolSizes(uint32_t maxSets) const;
		};

		struct vertex_input_info
		{
			std::array<vk::VertexInputBindingDescription, 1> _bindingDesc;
			std::vector<vk::VertexInputAttributeDescription> _attributeDesc;
		};

		using shared = std::shared_ptr<shader>;

		shader() = default;
		shader(shader&) = delete;
		shader(shader&&) = default;
		~shader();

		void create(const std::string_view inShaderPath);

		void destroy();

		bool isValid() const noexcept;

		std::string_view getPath() const noexcept;

		const SpvReflectShaderModule& getRefl() const;

		vk::PipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo() const noexcept;

		std::vector<descripted_data> getDescirptorShaderData() const noexcept;

		std::vector<vk::PushConstantRange> getPushConstantRanges() const noexcept;

		vertex_input_info getVertexInputInfo() const noexcept;

	protected:
		std::string _shaderPath;

		vk::ShaderModule _shaderModule;

		SpvReflectShaderModule _reflModule;
	};
} // namespace de::vulkan