#include "vk_descriptor_set.hxx"

#include "vk_allocator.hxx"
#include "vk_renderer.hxx"
#include "vk_texture_image.hxx"
#include "vk_utils.hxx"

vk_descriptor_set::vk_descriptor_set()
	: _vkDescriptorPool{VK_NULL_HANDLE}
{
}

vk_descriptor_set::~vk_descriptor_set()
{
}

void vk_descriptor_set::create(const std::vector<vk_graphics_pipeline*>& pipelines, const std::vector<vk_texture_image*>& textureImages)
{
	_pipelines = pipelines;

	const size_t descriptorSetsNum = _pipelines.size();
	VkDevice vkDevice = vk_renderer::get()->getDevice().get();

	createDescriptorPool(vkDevice, descriptorSetsNum);
	createUniformBuffer();

	std::vector<VkDescriptorBufferInfo> writeBufferInfos(descriptorSetsNum, VkDescriptorBufferInfo{});
	std::vector<VkDescriptorImageInfo> writeImageInfos(descriptorSetsNum, VkDescriptorImageInfo{});
	std::vector<VkWriteDescriptorSet> writeDescriptorSetInfo;
	writeDescriptorSetInfo.reserve(descriptorSetsNum * 2);

	_vkDescriptorSets.resize(descriptorSetsNum);

	for (size_t i = 0; i < descriptorSetsNum; ++i)
	{
		_vkDescriptorSets[i] = createDescriptorSet(vkDevice, _pipelines[i]->getDescriptorSetLayouts());
		createWriteForDescriptorSet(i, writeDescriptorSetInfo, writeBufferInfos[i], writeImageInfos[i], textureImages);
	}
	update(writeDescriptorSetInfo);
}

void vk_descriptor_set::rewrite(const std::pair<uint32_t, vk_texture_image*>& _textureImage)
{
	const size_t num = _pipelines.size();
	std::vector<VkDescriptorBufferInfo> writeBufferInfos(num, VkDescriptorBufferInfo{});
	std::vector<VkDescriptorImageInfo> writeImageInfos(num, VkDescriptorImageInfo{});
	std::vector<VkWriteDescriptorSet> writeDescriptorSetInfo;
	writeDescriptorSetInfo.reserve(num * 2);

	for (size_t i = 0; i < num; ++i)
	{
		if (_textureImage.first == _pipelines[i]->getMaterial()._baseColorTexture && _textureImage.second->isValid())
		{
			createWriteForDescriptorSet(i, writeDescriptorSetInfo, writeBufferInfos[i], writeImageInfos[i], _textureImage.second);
		}
	}
	update(writeDescriptorSetInfo);
}

void vk_descriptor_set::update(const std::vector<VkWriteDescriptorSet>& writeInfo)
{
	if (writeInfo.empty())
		return;

	VkDevice vkDevice = vk_renderer::get()->getDevice().get();
	vkUpdateDescriptorSets(vkDevice, writeInfo.size(), writeInfo.data(), 0, VK_NULL_HANDLE);
}

void vk_descriptor_set::bindToCmdBuffer(VkCommandBuffer commandBuffer)
{
	const size_t num = _vkDescriptorSets.size();
	for (size_t i = 0; i < num; ++i)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelines[i]->getLayout(), 0, 1, &_vkDescriptorSets[i], 0, nullptr);
	}
}

void vk_descriptor_set::destroy()
{
	if (VK_NULL_HANDLE != _vkDescriptorPool)
	{
		VkDevice vkDevice = vk_renderer::get()->getDevice().get();
		vkDestroyDescriptorPool(vkDevice, _vkDescriptorPool, vkGetAllocator());
		_vkDescriptorPool = VK_NULL_HANDLE;
		_uniformBuffer.destroy();
	}
}

const std::vector<VkDescriptorSet>& vk_descriptor_set::get() const
{
	return _vkDescriptorSets;
}

vk_buffer& vk_descriptor_set::getUniformBuffer()
{
	return _uniformBuffer;
}

void vk_descriptor_set::createDescriptorPool(const VkDevice vkDevice, const size_t count)
{
	VkDescriptorPoolSize uniformSize{};
	uniformSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformSize.descriptorCount = count;

	VkDescriptorPoolSize sampledImageSize{};
	sampledImageSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampledImageSize.descriptorCount = count;

	std::vector<VkDescriptorPoolSize> poolSizes{uniformSize, sampledImageSize};

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.pNext = nullptr;
	poolCreateInfo.flags = 0;
	poolCreateInfo.poolSizeCount = poolSizes.size();
	poolCreateInfo.pPoolSizes = poolSizes.data();
	poolCreateInfo.maxSets = count;

	VK_CHECK(vkCreateDescriptorPool(vkDevice, &poolCreateInfo, VK_NULL_HANDLE, &_vkDescriptorPool));
}

VkDescriptorSet vk_descriptor_set::createDescriptorSet(const VkDevice vkDevice, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = _vkDescriptorPool;
	allocInfo.descriptorSetCount = descriptorSetLayouts.size();
	allocInfo.pSetLayouts = descriptorSetLayouts.data();

	VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
	VK_CHECK(vkAllocateDescriptorSets(vkDevice, &allocInfo, &descriptorSet));
	return descriptorSet;
}

void vk_descriptor_set::createWriteForDescriptorSet(uint32_t index, std::vector<VkWriteDescriptorSet>& outWrite,
	VkDescriptorBufferInfo& exBufferInfo, VkDescriptorImageInfo& exImageInfo, const std::vector<vk_texture_image*>& textureImages)
{
	const uint32_t texImageIndex = _pipelines[index]->getMaterial()._baseColorTexture;
	const vk_texture_image* texImage = textureImages[texImageIndex];
	if (!texImage->isValid())
	{
		texImage = &vk_renderer::get()->getTextureImagePlaceholder();
	}

	createWriteForDescriptorSet(index, outWrite, exBufferInfo, exImageInfo, texImage);
}

void vk_descriptor_set::createWriteForDescriptorSet(uint32_t index, std::vector<VkWriteDescriptorSet>& outWrite,
	VkDescriptorBufferInfo& exBufferInfo, VkDescriptorImageInfo& exImageInfo, const vk_texture_image* texImage)
{
	exBufferInfo.buffer = _uniformBuffer.get();
	exBufferInfo.offset = 0;
	exBufferInfo.range = sizeof(uniforms);

	size_t lastWriteElem = outWrite.size();
	outWrite.resize(lastWriteElem + 2);

	outWrite[lastWriteElem].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	outWrite[lastWriteElem].pNext = nullptr;
	outWrite[lastWriteElem].dstSet = _vkDescriptorSets[index];
	outWrite[lastWriteElem].dstBinding = 0;
	outWrite[lastWriteElem].dstArrayElement = 0;
	outWrite[lastWriteElem].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	outWrite[lastWriteElem].descriptorCount = 1;
	outWrite[lastWriteElem].pBufferInfo = &exBufferInfo;
	outWrite[lastWriteElem].pImageInfo = nullptr;
	outWrite[lastWriteElem].pTexelBufferView = nullptr;

	exImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	exImageInfo.imageView = texImage->getImageView();
	exImageInfo.sampler = texImage->getSampler();

	++lastWriteElem;
	outWrite[lastWriteElem].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	outWrite[lastWriteElem].dstSet = _vkDescriptorSets[index];
	outWrite[lastWriteElem].dstBinding = 1;
	outWrite[lastWriteElem].dstArrayElement = 0;
	outWrite[lastWriteElem].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	outWrite[lastWriteElem].descriptorCount = 1;
	outWrite[lastWriteElem].pImageInfo = &exImageInfo;
}

void vk_descriptor_set::createUniformBuffer()
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffer_create_info.memory_properties_flags = vk_device_memory_properties::HOST;
	buffer_create_info.size = sizeof(uniforms);

	_uniformBuffer.create(buffer_create_info);
}