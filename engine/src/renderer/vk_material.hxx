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

protected:
	void createDescriptorSets(vk::Device device);

	void createGraphicsPipeline();

	std::vector<vk::WriteDescriptorSet> getDrescriptorWrites(const vk_shader& inShader);

	std::map<std::string, std::vector<vk::DescriptorBufferInfo>> getDescriptorBufferInfos(const vk_shader& inShader);
	std::map<std::string, std::vector<vk::DescriptorImageInfo>> getDescriptorImageInfos(const vk_shader& inShader);


private:
	const vk_shader::shared _vert;
	const vk_shader::shared _frag;

	vk::DescriptorPool _descriptorPool;
	std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts;
	std::vector<vk::DescriptorSet> _descriptorSets;
	std::vector<vk::DescriptorSetLayoutBinding> _descriptorBindings;

	vk_graphics_pipeline _pipeline;

	std::map<std::string, std::vector<vk_buffer*>> _buffers;
	std::map<std::string, std::vector<vk_texture_image*>> _images;
};