#pragma once

#include "images/vk_texture_image.hxx"

#include "vk_buffer.hxx"
#include "vk_graphics_pipeline.hxx"
#include "vk_shader.hxx"

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

class vk_material_instance final
{
	friend vk_material;
	vk_material_instance(class vk_material* owner);

public:
	~vk_material_instance() = default;

	vk::PipelineLayout getPipelineLayout() const;

	template <typename Str, typename Buf>
	void setBufferDependency(Str&& inName, Buf& inBuffer, size_t arrayIndex = 0);

	template <typename Str>
	void setBufferDependencySize(Str&& inName, size_t size);

	template <typename Str>
	void setImageDependecy(Str&& inName, const vk_texture_image* inImage, size_t arrayIndex = 0);

	template <typename Str>
	void setImageDependecySize(Str&& inName, size_t size);

	void updateDescriptorSets();

	void bindCmd(vk::CommandBuffer commandBuffer) const;

private:
	void updateShaderDescriptors(const vk_shader& inShader);
	std::map<std::string, std::vector<vk::DescriptorBufferInfo>> getDescriptorBufferInfos(const vk_shader& inShader) const;
	std::map<std::string, std::vector<vk::DescriptorImageInfo>> getDescriptorImageInfos(const vk_shader& inShader) const;

	class vk_material* _owner;

	std::vector<vk::DescriptorSet> _descriptorSets;

	std::map<std::string, std::vector<vk_buffer_region>> _buffers;
	std::map<std::string, std::vector<const vk_texture_image*>> _images;
	std::map<std::string, std::vector<vk_buffer*>> _ownedBuffers;
};

class vk_material final
{
	vk_material() = default;

public:
	using unique = std::unique_ptr<vk_material>;

	static unique makeNew(vk_shader::shared vert, vk_shader::shared frag, size_t maxInstances = 1);
	vk_material_instance& makeInstance();

	vk_material(vk_material&) = delete;
	vk_material(vk_material&&) = default;
	~vk_material();

	void recreatePipeline();

	void resizeDescriptorPool(size_t newSize);

	const std::vector<vk::DescriptorSetLayout>& getDescriptorSetLayouts() const;
	vk::DescriptorPool getDescriptorPool() const;

	std::vector<vk::PushConstantRange> getPushConstantRanges() const;
	std::vector<vk::PipelineShaderStageCreateInfo> getShaderStages() const;

	const vk_shader::shared& getVertShader() const;
	const vk_shader::shared& getFragShader() const;

	vk::PipelineLayout getPipelineLayout() const;
	const vk_graphics_pipeline& getPipeline() const;

private:
	void init(size_t maxInstances);
	void setShaderVert(const vk_shader::shared& inShader);
	void setShaderFrag(const vk_shader::shared& inShader);
	void setInstanceCount(uint32_t inValue);

	void createDescriptorPool(uint32_t maxSets = 1);

	vk_shader::shared _vert;
	vk_shader::shared _frag;
	
	std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts;
	vk::DescriptorPool _descriptorPool;

	vk_graphics_pipeline _pipeline;

	std::vector<vk_material_instance> _instances;
};

template <typename Str, typename Buf>
void vk_material_instance::setBufferDependency(Str&& inName, Buf& inBuffer, size_t arrayIndex)
{
	auto it = _buffers.try_emplace(std::forward<Str>(inName), std::vector<vk_buffer_region>(1, vk_buffer_region()));
	it.first->second[arrayIndex] = vk_buffer_region(std::forward<Buf>(inBuffer));
}

template <typename Str>
void vk_material_instance::setBufferDependencySize(Str&& inName, size_t size)
{
	auto& arr = _buffers[std::forward<Str>(inName)];
	if (arr.size() != size)
	{
		arr.resize(size);
	}
}

template <typename Str>
void vk_material_instance::setImageDependecy(Str&& inName, const vk_texture_image* inImage, size_t arrayIndex)
{
	auto it = _images.try_emplace(std::forward<Str>(inName), std::vector<const vk_texture_image*>(1, nullptr));
	it.first->second[arrayIndex] = inImage;
}

template <typename Str>
void vk_material_instance::setImageDependecySize(Str&& inName, size_t size)
{
	auto& arr = _images[std::forward<Str>(inName)];
	if (arr.size() != size)
	{
		arr.resize(size);
	}
}