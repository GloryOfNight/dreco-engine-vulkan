#include "material_instance.hxx"

#include "renderer.hxx"

de::vulkan::material_instance::material_instance(material* owner)
{
	_owner = owner;
	allocate();
}

void de::vulkan::material_instance::allocate()
{
	auto device = renderer::get()->getDevice();
	_descriptorSets = device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo()
														.setDescriptorPool(_owner->getDescriptorPool())
														.setSetLayouts(_owner->getDescriptorSetLayouts()));
}

void de::vulkan::material_instance::free()
{
	auto device = renderer::get()->getDevice();
	device.freeDescriptorSets(_owner->getDescriptorPool(), _descriptorSets);
	_descriptorSets.clear();
}

de::vulkan::material* de::vulkan::material_instance::getMaterial() const
{
	return _owner;
}

void de::vulkan::material_instance::updateShaderDescriptors(const shader& inShader)
{
	std::vector<vk::WriteDescriptorSet> writes;

	const auto descriptorBufferInfos = getDescriptorBufferInfos(inShader);
	const auto descriptorImageInfos = getDescriptorImageInfos(inShader);

	const auto& relf = inShader.getRefl();
	for (uint8_t i = 0; i < relf.descriptor_set_count; i++)
	{
		const auto& reflDescSet = relf.descriptor_sets[i];
		for (uint8_t k = 0; k < reflDescSet.binding_count; k++)
		{
			const auto& reflBinding = relf.descriptor_bindings[k];

			vk::WriteDescriptorSet& write = writes.emplace_back(vk::WriteDescriptorSet())
												.setDstSet(_descriptorSets[reflDescSet.set])
												.setDstBinding(reflBinding.binding)
												.setDescriptorCount(reflBinding.count)
												.setDescriptorType(static_cast<vk::DescriptorType>(reflBinding.descriptor_type));
			switch (write.descriptorType)
			{
			case vk::DescriptorType::eUniformBuffer:
				write.setPBufferInfo(descriptorBufferInfos.at(reflBinding.name).data());
				break;
			case vk::DescriptorType::eCombinedImageSampler:
				write.setPImageInfo(descriptorImageInfos.at(reflBinding.name).data());
				break;
			default:
				break;
			}
		}
	}
	renderer::get()->getDevice().updateDescriptorSets(writes, {});
}

std::map<std::string, std::vector<vk::DescriptorBufferInfo>> de::vulkan::material_instance::getDescriptorBufferInfos(const shader& inShader) const
{
	std::map<std::string, std::vector<vk::DescriptorBufferInfo>> out;

	const auto& relf = inShader.getRefl();

	for (uint8_t i = 0; i < relf.descriptor_binding_count; i++)
	{
		const auto& reflBinding = relf.descriptor_bindings[i];
		if (reflBinding.descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			continue;
		}

		const auto descBufferInfo = out.emplace(reflBinding.name, std::vector<vk::DescriptorBufferInfo>());
		descBufferInfo.first->second.reserve(reflBinding.count);

		const auto& bufferBind = _buffers.at(reflBinding.name);
		for (uint32_t k = 0; k < reflBinding.count; ++k)
		{
			const auto buffer = bufferBind[k];
			auto info = vk::DescriptorBufferInfo()
							.setBuffer(buffer->get())
							.setOffset(0)
							.setRange(buffer->getSize());
			descBufferInfo.first->second.push_back(info);
		}
	}

	return out;
}

std::map<std::string, std::vector<vk::DescriptorImageInfo>> de::vulkan::material_instance::getDescriptorImageInfos(const shader& inShader) const
{
	std::map<std::string, std::vector<vk::DescriptorImageInfo>> out;

	const auto& relf = inShader.getRefl();

	for (uint8_t i = 0; i < relf.descriptor_binding_count; i++)
	{
		const auto& reflBinding = relf.descriptor_bindings[i];
		if (reflBinding.descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		{
			continue;
		}

		const auto descBufferInfo = out.emplace(reflBinding.name, std::vector<vk::DescriptorImageInfo>());
		descBufferInfo.first->second.reserve(reflBinding.count);

		const auto& bindIt = _images.find(reflBinding.name);
		for (uint32_t k = 0; k < reflBinding.count; ++k)
		{
			const auto image = bindIt != _images.end() ? bindIt->second[k] : &renderer::get()->getTextureImagePlaceholder();
			auto info = vk::DescriptorImageInfo()
							.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
							.setImageView(image->getImageView())
							.setSampler(image->getSampler());
			descBufferInfo.first->second.push_back(info);
		}
	}
	return out;
}

void de::vulkan::material_instance::updateDescriptorSets()
{
	updateShaderDescriptors(*_owner->getVertShader());
	updateShaderDescriptors(*_owner->getFragShader());
}

void de::vulkan::material_instance::bindCmd(vk::CommandBuffer commandBuffer) const
{
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _owner->getPipelineLayout(), 0, _descriptorSets, nullptr);
}