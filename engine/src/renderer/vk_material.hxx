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

class vk_material final
{
public:
	vk_material() = default;
	vk_material(vk_material&) = delete;
	vk_material(vk_material&&) = default;
	~vk_material();

	void init();

	void setShaderVert(const vk_shader::shared& inShader);
	void setShaderFrag(const vk_shader::shared& inShader);

	void recreatePipeline();
	void updateDescriptorSets();

	template <typename Str, typename Buf>
	void setBufferDependency(Str&& inName, Buf& inBuffer, size_t arrayIndex = 0);

	template <typename Str>
	void setBufferDependencySize(Str&& inName, size_t size);

	template <typename Str>
	void setImageDependecy(Str&& inName, const vk_texture_image* inImage, size_t arrayIndex = 0);

	template <typename Str>
	void setImageDependecySize(Str&& inName, size_t size);

	void bindCmd(vk::CommandBuffer commandBuffer);

	const std::vector<vk::DescriptorSetLayout>& getDescriptorSetLayouts() const;
	std::vector<vk::PushConstantRange> getPushConstantRanges() const;
	std::vector<vk::PipelineShaderStageCreateInfo> getShaderStages() const;

	const vk_shader::shared& getVertShader() const;
	const vk_shader::shared& getFragShader() const;

	vk::PipelineLayout getPipelineLayout() const;

protected:
	void createDescriptorSets();

	void updateShaderDescriptors(const vk_shader& inShader);
	std::map<std::string, std::vector<vk::DescriptorBufferInfo>> getDescriptorBufferInfos(const vk_shader& inShader) const;
	std::map<std::string, std::vector<vk::DescriptorImageInfo>> getDescriptorImageInfos(const vk_shader& inShader) const;

private:
	vk_shader::shared _vert;
	vk_shader::shared _frag;

	vk::DescriptorPool _descriptorPool;
	std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts;
	std::vector<vk::DescriptorSet> _descriptorSets;

	vk_graphics_pipeline _pipeline;

	std::map<std::string, std::vector<vk_buffer_region>> _buffers;
	std::map<std::string, std::vector<const vk_texture_image*>> _images;

	std::map<std::string, std::vector<vk_buffer*>> _ownedBuffers;
};

template <typename Str, typename Buf>
void vk_material::setBufferDependency(Str&& inName, Buf& inBuffer, size_t arrayIndex)
{
	auto it = _buffers.try_emplace(std::forward<Str>(inName), std::vector<vk_buffer_region>(1, vk_buffer_region()));
	it.first->second[arrayIndex] = vk_buffer_region(std::forward<Buf>(inBuffer));
}

template <typename Str>
void vk_material::setBufferDependencySize(Str&& inName, size_t size)
{
	auto& arr = _buffers[std::forward<Str>(inName)];
	if (arr.size() != size)
	{
		arr.resize(size);
	}
}

template <typename Str>
void vk_material::setImageDependecy(Str&& inName, const vk_texture_image* inImage, size_t arrayIndex)
{
	auto it = _images.try_emplace(std::forward<Str>(inName), std::vector<const vk_texture_image*>(1, nullptr));
	it.first->second[arrayIndex] = inImage;
}

template <typename Str>
void vk_material::setImageDependecySize(Str&& inName, size_t size)
{
	auto& arr = _images[std::forward<Str>(inName)];
	if (arr.size() != size)
	{
		arr.resize(size);
	}
}