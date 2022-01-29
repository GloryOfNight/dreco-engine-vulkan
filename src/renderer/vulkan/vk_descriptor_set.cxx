#include "vk_descriptor_set.hxx"

#include "vk_renderer.hxx"
#include "vk_texture_image.hxx"
#include "vk_utils.hxx"

void vk_descriptor_set::create(const std::vector<vk_graphics_pipeline*>& pipelines, const std::vector<vk_texture_image*>& textureImages)
{
	_pipelines = pipelines;

	const size_t descriptorSetsNum = _pipelines.size();
	const vk::Device device = vk_renderer::get()->getDevice();

	createDescriptorPool(device, descriptorSetsNum);
	createUniformBuffer();

	std::vector<vk::DescriptorBufferInfo> writeBufferInfos(descriptorSetsNum, vk::DescriptorBufferInfo());
	std::vector<vk::DescriptorImageInfo> writeImageInfos(descriptorSetsNum, vk::DescriptorImageInfo());
	std::vector<vk::WriteDescriptorSet> writeDescriptorSetInfo;
	writeDescriptorSetInfo.reserve(descriptorSetsNum * 2);

	_descriptorSets.resize(descriptorSetsNum);

	for (size_t i = 0; i < descriptorSetsNum; ++i)
	{
		_descriptorSets[i] = createDescriptorSet(device, _pipelines[i]->getDescriptorSetLayouts());
		createWriteForDescriptorSet(i, writeDescriptorSetInfo, writeBufferInfos[i], writeImageInfos[i], textureImages);
	}
	update(writeDescriptorSetInfo);
}

void vk_descriptor_set::rewrite(const std::pair<uint32_t, vk_texture_image*>& textureImage)
{
	const size_t num = _pipelines.size();
	std::vector<vk::DescriptorBufferInfo> writeBufferInfos(num, vk::DescriptorBufferInfo());
	std::vector<vk::DescriptorImageInfo> writeImageInfos(num, vk::DescriptorImageInfo());
	std::vector<vk::WriteDescriptorSet> writeDescriptorSetInfo;
	writeDescriptorSetInfo.reserve(num * 2);

	for (size_t i = 0; i < num; ++i)
	{
		if (textureImage.first == _pipelines[i]->getMaterial().pbrMetallicRoughness._baseColorTexture._index && textureImage.second->isValid())
		{
			createWriteForDescriptorSet(i, writeDescriptorSetInfo, writeBufferInfos[i], writeImageInfos[i], textureImage.second);
		}
	}
	update(writeDescriptorSetInfo);
}

void vk_descriptor_set::update(const std::vector<vk::WriteDescriptorSet>& writeInfo)
{
	if (writeInfo.empty())
		return;

	const vk::Device device = vk_renderer::get()->getDevice();
	device.updateDescriptorSets(writeInfo, nullptr);
}

void vk_descriptor_set::bindToCmdBuffer(vk::CommandBuffer commandBuffer)
{
	const size_t num = _descriptorSets.size();
	for (size_t i = 0; i < num; ++i)
	{
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelines[i]->getLayout(), 0, _descriptorSets, nullptr);
	}
}

void vk_descriptor_set::destroy()
{
	if (_descriptorPool)
	{
		const vk::Device device = vk_renderer::get()->getDevice();
		device.destroyDescriptorPool(_descriptorPool);
		_descriptorPool = nullptr;

		_uniformBuffer.destroy();
	}
}

vk_buffer& vk_descriptor_set::getUniformBuffer()
{
	return _uniformBuffer;
}

void vk_descriptor_set::createDescriptorPool(const vk::Device device, const size_t count)
{
	const vk::DescriptorPoolSize uniformSize =
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, count);

	const vk::DescriptorPoolSize sampledImageSize =
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, count);

	const std::vector<vk::DescriptorPoolSize> poolSizes{uniformSize, sampledImageSize};

	const vk::DescriptorPoolCreateInfo poolCreateInfo =
		vk::DescriptorPoolCreateInfo()
			.setPoolSizes(poolSizes)
			.setMaxSets(count);

	_descriptorPool = device.createDescriptorPool(poolCreateInfo);
}

vk::DescriptorSet vk_descriptor_set::createDescriptorSet(const vk::Device device, const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts)
{
	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo =
		vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(_descriptorPool)
			.setSetLayouts(descriptorSetLayouts);

	return device.allocateDescriptorSets(descriptorSetAllocateInfo)[0];
}

void vk_descriptor_set::createWriteForDescriptorSet(uint32_t index, std::vector<vk::WriteDescriptorSet>& outWrite,
	vk::DescriptorBufferInfo& exBufferInfo, vk::DescriptorImageInfo& exImageInfo, const std::vector<vk_texture_image*>& textureImages)
{
	uint32_t texImageIndex = 0;
	if (const auto pbrTexImage = _pipelines[index]->getMaterial().pbrMetallicRoughness._baseColorTexture._index; pbrTexImage != UINT32_MAX)
	{
		texImageIndex = pbrTexImage;
	}
	else if (const auto emissiveTexture = _pipelines[index]->getMaterial()._emissiveTexture._index; emissiveTexture != UINT32_MAX)
	{
		texImageIndex = emissiveTexture;
	}

	const vk_texture_image* texImage = textureImages[texImageIndex];
	if (!texImage->isValid())
	{
		texImage = &vk_renderer::get()->getTextureImagePlaceholder();
	}

	createWriteForDescriptorSet(index, outWrite, exBufferInfo, exImageInfo, texImage);
}

void vk_descriptor_set::createWriteForDescriptorSet(uint32_t index, std::vector<vk::WriteDescriptorSet>& outWrite,
	vk::DescriptorBufferInfo& exBufferInfo, vk::DescriptorImageInfo& exImageInfo, const vk_texture_image* texImage)
{
	exBufferInfo.buffer = _uniformBuffer.get();
	exBufferInfo.offset = 0;
	exBufferInfo.range = sizeof(uniforms);

	size_t lastWriteElem = outWrite.size();
	outWrite.resize(lastWriteElem + 2);

	outWrite[lastWriteElem].dstSet = _descriptorSets[index];
	outWrite[lastWriteElem].dstBinding = 0;
	outWrite[lastWriteElem].dstArrayElement = 0;
	outWrite[lastWriteElem].descriptorType = vk::DescriptorType::eUniformBuffer;
	outWrite[lastWriteElem].descriptorCount = 1;
	outWrite[lastWriteElem].pBufferInfo = &exBufferInfo;
	outWrite[lastWriteElem].pImageInfo = nullptr;
	outWrite[lastWriteElem].pTexelBufferView = nullptr;

	exImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	exImageInfo.imageView = texImage->getImageView();
	exImageInfo.sampler = texImage->getSampler();

	++lastWriteElem;
	outWrite[lastWriteElem].dstSet = _descriptorSets[index];
	outWrite[lastWriteElem].dstBinding = 1;
	outWrite[lastWriteElem].dstArrayElement = 0;
	outWrite[lastWriteElem].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	outWrite[lastWriteElem].descriptorCount = 1;
	outWrite[lastWriteElem].pImageInfo = &exImageInfo;
}

void vk_descriptor_set::createUniformBuffer()
{
	vk_buffer::create_info bufferCreateInfo{};
	bufferCreateInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	bufferCreateInfo.memoryPropertiesFlags = vk_buffer::create_info::hostMemoryPropertiesFlags;
	bufferCreateInfo.size = sizeof(uniforms);

	_uniformBuffer.create(bufferCreateInfo);
}