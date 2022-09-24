#include "vk_material.hxx"

void vk_material::init()
{
}

void vk_material::createDescriptorSets(vk::Device device)
{
	std::vector<vk_descriptor_shader_data> shadersDataSets;
	{
		auto dataSets = _vert->getDescirptorShaderData();
		std::move(dataSets.begin(), dataSets.end(), std::back_inserter(shadersDataSets));
	}
	{
		auto dataSets = _frag->getDescirptorShaderData();
		std::move(dataSets.begin(), dataSets.end(), std::back_inserter(shadersDataSets));
	}

	std::vector<vk::DescriptorPoolSize> poolSizes;
	const auto& shaderDataSets = _vert->getDescirptorShaderData();
	for (const auto& data : shaderDataSets)
	{
		_descriptorSetLayouts.push_back(device.createDescriptorSetLayout(data._descriptorSetLayoutCreateInfo));

		auto& dataPoolSizes = data.getDescriptorPoolSizes();
		std::move(dataPoolSizes.begin(), dataPoolSizes.end(), std::back_inserter(poolSizes));

		std::move(data._descriptorSetLayoutBindings.begin(), data._descriptorSetLayoutBindings.end(), std::back_inserter(_descriptorBindings));
	}

	_descriptorPool = device.createDescriptorPool(vk::DescriptorPoolCreateInfo()
													  .setPoolSizes(poolSizes)
													  .setMaxSets(_descriptorSetLayouts.size()));

	_descriptorSets = device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo()
														.setDescriptorPool(_descriptorPool)
														.setSetLayouts(_descriptorSetLayouts));
}

void vk_material::createGraphicsPipeline()
{
}

std::vector<vk::WriteDescriptorSet> vk_material::getDrescriptorWrites(const vk_shader& inShader)
{
	std::vector<vk::WriteDescriptorSet> out;

	const auto descriptorBufferInfos = getDescriptorBufferInfos(inShader);
	const auto descriptorImageInfos = getDescriptorImageInfos(inShader);

	const auto& relf = inShader.getRefl();
	for (uint8_t i = 0; i < relf.descriptor_set_count; i++)
	{
		const auto& reflDescSet = relf.descriptor_sets[i];
		for (uint8_t k = 0; k < reflDescSet.binding_count; k++)
		{
			const auto& reflBinding = relf.descriptor_bindings[k];

			vk::WriteDescriptorSet write = vk::WriteDescriptorSet()
											   .setDstSet(_descriptorSets[reflDescSet.set])
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
	return out;
}

std::map<std::string, std::vector<vk::DescriptorBufferInfo>> vk_material::getDescriptorBufferInfos(const vk_shader& inShader)
{
	std::map<std::string, std::vector<vk::DescriptorBufferInfo>> out;

	const auto& relf = inShader.getRefl();

	for (uint8_t i = 0; i < relf.descriptor_binding_count; i++)
	{
		const auto& reflBinding = relf.descriptor_bindings[i];
		auto& descBufferInfo = out.emplace(reflBinding.name);
		descBufferInfo.first->second.reserve(reflBinding.count);

		const auto& bufferBind = _buffers[reflBinding.name];
		for (uint32_t k = 0; k < reflBinding.count; ++k)
		{
			const auto buffer = bufferBind[k];
			auto info = vk::DescriptorBufferInfo()
							.setBuffer(buffer->get())
							.setOffset(buffer->getOffset())
							.setRange(buffer->getSize());
			descBufferInfo.first->second.push_back(info);
		}
	}

	return out;
}

std::map<std::string, std::vector<vk::DescriptorImageInfo>> vk_material::getDescriptorImageInfos(const vk_shader& inShader)
{
	std::map<std::string, std::vector<vk::DescriptorImageInfo>> out;

	const auto& relf = inShader.getRefl();

	for (uint8_t i = 0; i < relf.descriptor_binding_count; i++)
	{
		const auto& reflBinding = relf.descriptor_bindings[i];
		auto& descBufferInfo = out.emplace(reflBinding.name);
		descBufferInfo.first->second.reserve(reflBinding.count);

		const auto& bind = _images[reflBinding.name];
		for (uint32_t k = 0; k < reflBinding.count; ++k)
		{
			const auto image = bind[k];
			auto info = vk::DescriptorImageInfo()
							.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
							.setImageView(image->getImageView())
							.setSampler(image->getSampler());
			descBufferInfo.first->second.push_back(info);
		}
	}
	return out;
}
