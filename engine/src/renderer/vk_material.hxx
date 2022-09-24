#pragma once

#include "vk_buffer.hxx"
#include "vk_graphics_pipeline.hxx"
#include "vk_shader.hxx"
#include "vk_texture_image.hxx"

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

class vk_material
{
public:
	void init();

	void setShaderVert(const vk_shader::shared& inShader);
	void setShaderFrag(const vk_shader::shared& inShader);

	void addBufferDependecy(const std::string_view& inName, vk_buffer* inBuffer, size_t arrayIndex = 0);
	void addBufferDependecy(const std::string_view& inName, std::vector<vk_buffer*> inBuffers);

	void addImageDependecy(const std::string_view& inName, vk_texture_image* inImage, size_t arrayIndex = 0);
	void addImageDependecy(const std::string_view& inName, std::vector<vk_texture_image*> inImages);

protected:
	void createDescriptorSets();

	void createGraphicsPipeline();

	std::vector<vk::WriteDescriptorSet> getDrescriptorWrites(const vk_shader& inShader);

	std::map<std::string, std::vector<vk::DescriptorBufferInfo>> getDescriptorBufferInfos(const vk_shader& inShader);
	std::map<std::string, std::vector<vk::DescriptorImageInfo>> getDescriptorImageInfos(const vk_shader& inShader);

private:
	vk_shader::shared _vert;
	vk_shader::shared _frag;

	vk::DescriptorPool _descriptorPool;
	std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts;
	std::vector<vk::DescriptorSet> _descriptorSets;
	std::vector<vk::DescriptorSetLayoutBinding> _descriptorBindings;

	vk_graphics_pipeline _pipeline;

	std::map<std::string, std::vector<vk_buffer*>> _buffers;
	std::map<std::string, std::vector<vk_texture_image*>> _images;
};