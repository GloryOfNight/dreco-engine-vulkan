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

	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	for (int i = 0; i < descriptorSetsNum; ++i)
	{
		descriptorSetLayouts.push_back(_pipelines[i]->getDescriptorSetLayout());
	}
	_descriptorSets = createDescriptorSets(device, descriptorSetLayouts);

	vk_descriptor_set_write write;
	for (size_t i = 0; i < descriptorSetsNum; ++i)
	{
		addWriteBufferInfo(write, _uniformBuffer);

		const auto& pipelineMaterial = _pipelines[i]->getMaterial();
		const auto& placeholderTexture = vk_renderer::get()->getTextureImagePlaceholder();

		const uint32_t emissiveTextureTextureIndex = pipelineMaterial._emissiveTexture._index;
		const auto& emissiveTexture = emissiveTextureTextureIndex != UINT32_MAX && textureImages[emissiveTextureTextureIndex]->isValid()
										  ? *textureImages[emissiveTextureTextureIndex]
										  : placeholderTexture;
		writeDescriptorSetsBufferInfos(i, write);

		addWriteImageInfo(write, emissiveTexture);

		const uint32_t baseColorTextureIndex = pipelineMaterial.pbrMetallicRoughness._baseColorTexture._index;
		const auto& baseColorTexture = baseColorTextureIndex != UINT32_MAX && textureImages[baseColorTextureIndex]->isValid()
										   ? *textureImages[baseColorTextureIndex]
										   : placeholderTexture;
		addWriteImageInfo(write, baseColorTexture);

		const uint32_t metallicRoughnessTextureIndex = pipelineMaterial.pbrMetallicRoughness._metallicRoughnessTexture._index;
		const auto& metallicRoughnessTexture = metallicRoughnessTextureIndex != UINT32_MAX && textureImages[metallicRoughnessTextureIndex]->isValid()
												   ? *textureImages[metallicRoughnessTextureIndex]
												   : placeholderTexture;
		addWriteImageInfo(write, metallicRoughnessTexture);

		const uint32_t normalTextureIndex = pipelineMaterial._normalTexture._index;
		const auto& normalTexture = normalTextureIndex != UINT32_MAX && textureImages[normalTextureIndex]->isValid()
										? *textureImages[normalTextureIndex]
										: placeholderTexture;
		addWriteImageInfo(write, normalTexture);

		writeDescriptorSetsImageInfos(i, write);
	}
	update(write.descriptorSets);
}

void vk_descriptor_set::updateTextureImages(const std::vector<vk_texture_image*>& textureImages)
{
	const size_t totalPipelines = _pipelines.size();

	vk_descriptor_set_write write;
	for (size_t i = 0; i < totalPipelines; ++i)
	{
		const auto& pipelineMaterial = _pipelines[i]->getMaterial();
		const auto& placeholderTexture = vk_renderer::get()->getTextureImagePlaceholder();

		const uint32_t baseColorTextureIndex = pipelineMaterial.pbrMetallicRoughness._baseColorTexture._index;
		const auto& baseColorTexture = baseColorTextureIndex != UINT32_MAX && textureImages[baseColorTextureIndex]->isValid()
										   ? *textureImages[baseColorTextureIndex]
										   : placeholderTexture;
		addWriteImageInfo(write, baseColorTexture);

		const uint32_t metallicRoughnessTextureIndex = pipelineMaterial.pbrMetallicRoughness._metallicRoughnessTexture._index;
		const auto& metallicRoughnessTexture = metallicRoughnessTextureIndex != UINT32_MAX && textureImages[metallicRoughnessTextureIndex]->isValid()
												   ? *textureImages[metallicRoughnessTextureIndex]
												   : placeholderTexture;
		addWriteImageInfo(write, metallicRoughnessTexture);

		const uint32_t normalTextureIndex = pipelineMaterial._normalTexture._index;
		const auto& normalTexture = normalTextureIndex != UINT32_MAX && textureImages[normalTextureIndex]->isValid()
										? *textureImages[normalTextureIndex]
										: placeholderTexture;
		addWriteImageInfo(write, normalTexture);

		const uint32_t emissiveTextureTextureIndex = pipelineMaterial._emissiveTexture._index;
		const auto& emissiveTexture = emissiveTextureTextureIndex != UINT32_MAX && textureImages[emissiveTextureTextureIndex]->isValid()
										  ? *textureImages[emissiveTextureTextureIndex]
										  : placeholderTexture;

		addWriteImageInfo(write, emissiveTexture);

		writeDescriptorSetsImageInfos(i, write);
	}

	update(write.descriptorSets);
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
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelines[i]->getLayout(), 0, _descriptorSets[i], nullptr);
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

std::vector<vk::DescriptorSet> vk_descriptor_set::createDescriptorSets(const vk::Device device, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
{
	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo =
		vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(_descriptorPool)
			.setSetLayouts(descriptorSetLayouts);

	return device.allocateDescriptorSets(descriptorSetAllocateInfo);
}

vk::DescriptorBufferInfo& vk_descriptor_set::addWriteBufferInfo(vk_descriptor_set_write& write, const vk_buffer& buffer)
{
	auto& bufferInfo = write.bufferInfos.emplace_back(vk::DescriptorBufferInfo());
	bufferInfo.buffer = _uniformBuffer.get();
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(uniforms);
	return bufferInfo;
}

vk::DescriptorImageInfo& vk_descriptor_set::addWriteImageInfo(vk_descriptor_set_write& write, const vk_texture_image& image)
{
	auto& imageInfo = write.imageInfos.emplace_back(vk::DescriptorImageInfo());
	imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	imageInfo.imageView = image.getImageView();
	imageInfo.sampler = image.getSampler();
	return imageInfo;
}

void vk_descriptor_set::writeDescriptorSetsBufferInfos(uint32_t index, vk_descriptor_set_write& write)
{
	auto& uniformBufferWrite = write.descriptorSets.emplace_back(vk::WriteDescriptorSet());
	uniformBufferWrite.dstSet = _descriptorSets[index];
	uniformBufferWrite.dstBinding = 0;
	uniformBufferWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
	uniformBufferWrite.descriptorCount = write.bufferInfos.size();
	uniformBufferWrite.pBufferInfo = write.bufferInfos.data();
}

void vk_descriptor_set::writeDescriptorSetsImageInfos(uint32_t index, vk_descriptor_set_write& write)
{
	auto& imageSamplerWrite = write.descriptorSets.emplace_back(vk::WriteDescriptorSet{});
	imageSamplerWrite.dstSet = _descriptorSets[index];
	imageSamplerWrite.dstBinding = 1;
	imageSamplerWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	imageSamplerWrite.descriptorCount = write.imageInfos.size();
	imageSamplerWrite.pImageInfo = write.imageInfos.data();
}

void vk_descriptor_set::createUniformBuffer()
{
	vk_buffer::create_info bufferCreateInfo{};
	bufferCreateInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	bufferCreateInfo.memoryPropertiesFlags = vk_buffer::create_info::hostMemoryPropertiesFlags;
	bufferCreateInfo.size = sizeof(uniforms);

	_uniformBuffer.create(bufferCreateInfo);
}