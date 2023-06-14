#pragma once

#include "images/texture_image.hxx"

#include "buffer.hxx"
#include "graphics_pipeline.hxx"
#include "shader.hxx"

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class material_instance final
	{
		friend material;
		material_instance(class material* owner);

	public:
		using unique = std::unique_ptr<material_instance>;

		material_instance(material_instance&) = delete;
		material_instance(material_instance&&) = default;
		~material_instance() = default;

		vk::PipelineLayout getPipelineLayout() const;

		template <typename Str>
		void setBufferDependency(Str&& inName, const de::vulkan::buffer* inBuffer, size_t arrayIndex = 0);

		template <typename Str>
		void setBufferDependencySize(Str&& inName, size_t size);

		template <typename Str>
		void setImageDependecy(Str&& inName, const texture_image* inImage, size_t arrayIndex = 0);

		template <typename Str>
		void setImageDependecySize(Str&& inName, size_t size);

		void updateDescriptorSets();

		void bindCmd(vk::CommandBuffer commandBuffer) const;

	private:
		void updateShaderDescriptors(const shader& inShader);
		std::map<std::string, std::vector<vk::DescriptorBufferInfo>> getDescriptorBufferInfos(const shader& inShader) const;
		std::map<std::string, std::vector<vk::DescriptorImageInfo>> getDescriptorImageInfos(const shader& inShader) const;

		class material* _owner;

		std::vector<vk::DescriptorSet> _descriptorSets;

		std::map<std::string, std::vector<const de::vulkan::buffer*>> _buffers;
		std::map<std::string, std::vector<const texture_image*>> _images;
	};

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

		const std::vector<vk::DescriptorSetLayout>& getDescriptorSetLayouts() const;
		vk::DescriptorPool getDescriptorPool() const;

		std::vector<vk::PushConstantRange> getPushConstantRanges() const;
		std::vector<vk::PipelineShaderStageCreateInfo> getShaderStages() const;

		const shader::shared& getVertShader() const;
		const shader::shared& getFragShader() const;

		vk::PipelineLayout getPipelineLayout() const;
		const graphics_pipeline& getPipeline() const;

	private:
		void init(size_t maxInstances);
		void setShaderVert(const shader::shared& inShader);
		void setShaderFrag(const shader::shared& inShader);
		void setInstanceCount(uint32_t inValue);

		void createDescriptorPool(uint32_t maxSets = 1);

		void createPipelineLayout();

		shader::shared _vert;
		shader::shared _frag;

		std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts;
		vk::DescriptorPool _descriptorPool;

		vk::PipelineLayout _pipelineLayout;
		graphics_pipeline _pipeline;

		std::vector<material_instance::unique> _instances;
	};

	template <typename Str>
	void material_instance::setBufferDependency(Str&& inName, const de::vulkan::buffer* inBuffer, size_t arrayIndex)
	{
		auto it = _buffers.try_emplace(std::forward<Str>(inName), std::vector<const de::vulkan::buffer*>(1, nullptr));
		it.first->second[arrayIndex] = inBuffer;
	}

	template <typename Str>
	void material_instance::setBufferDependencySize(Str&& inName, size_t size)
	{
		auto& arr = _buffers[std::forward<Str>(inName)];
		if (arr.size() != size)
		{
			arr.resize(size);
		}
	}

	template <typename Str>
	void material_instance::setImageDependecy(Str&& inName, const texture_image* inImage, size_t arrayIndex)
	{
		auto it = _images.try_emplace(std::forward<Str>(inName), std::vector<const texture_image*>(1, nullptr));
		it.first->second[arrayIndex] = inImage;
	}

	template <typename Str>
	void material_instance::setImageDependecySize(Str&& inName, size_t size)
	{
		auto& arr = _images[std::forward<Str>(inName)];
		if (arr.size() != size)
		{
			arr.resize(size);
		}
	}
} // namespace de::vulkan