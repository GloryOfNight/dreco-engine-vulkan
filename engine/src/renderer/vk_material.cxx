#include "vk_material.hxx"

#include "vk_renderer.hxx"

vk_material::~vk_material()
{
	auto device = vk_renderer::get()->getDevice();
	if (_descriptorPool)
	{
		for (auto layout : _descriptorSetLayouts)
			device.destroyDescriptorSetLayout(layout);
		device.destroyDescriptorPool(_descriptorPool);
	}
}

void vk_material::init()
{
	createDescriptorSets();
	_pipeline.create(*this);
}

void vk_material::setShaderVert(const vk_shader::shared& inShader)
{
	_vert = inShader;
}

void vk_material::setShaderFrag(const vk_shader::shared& inShader)
{
	_frag = inShader;
}

void vk_material::recreatePipeline()
{
	_pipeline.recreatePipeline();
}

void vk_material::updateDescriptorSets()
{
	updateShaderDescriptors(*_vert);
	updateShaderDescriptors(*_frag);
}

void vk_material::bindCmd(vk::CommandBuffer commandBuffer)
{
	_pipeline.bindCmd(commandBuffer);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipeline.getLayout(), 0, _descriptorSets, nullptr);
}

const std::vector<vk::DescriptorSetLayout>& vk_material::getDescriptorSetLayouts() const
{
	return _descriptorSetLayouts;
}

std::vector<vk::PushConstantRange> vk_material::getPushConstantRanges() const
{
	std::vector<vk::PushConstantRange> out;
	{
		auto ranges = _vert->getPushConstantRanges();
		std::move(ranges.begin(), ranges.end(), std::back_inserter(out));
	}
	{
		auto ranges = _frag->getPushConstantRanges();
		std::move(ranges.begin(), ranges.end(), std::back_inserter(out));
	}
	return out;
}

std::vector<vk::PipelineShaderStageCreateInfo> vk_material::getShaderStages() const
{
	auto out = std::vector<vk::PipelineShaderStageCreateInfo>(2, vk::PipelineShaderStageCreateInfo());
	out[0] = _vert->getPipelineShaderStageCreateInfo();
	out[1] = _frag->getPipelineShaderStageCreateInfo();
	return out;
}

const vk_shader::shared& vk_material::getVertShader() const
{
	return _vert;
}

const vk_shader::shared& vk_material::getFragShader() const
{
	return _frag;
}

vk::PipelineLayout vk_material::getPipelineLayout() const
{
	return _pipeline.getLayout();
}

void vk_material::createDescriptorSets()
{
	auto device = vk_renderer::get()->getDevice();

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
	for (const auto& data : shadersDataSets)
	{
		_descriptorSetLayouts.push_back(device.createDescriptorSetLayout(data._descriptorSetLayoutCreateInfo));

		const auto dataPoolSizes = data.getDescriptorPoolSizes();
		std::move(dataPoolSizes.begin(), dataPoolSizes.end(), std::back_inserter(poolSizes));
	}

	_descriptorPool = device.createDescriptorPool(vk::DescriptorPoolCreateInfo()
													  .setPoolSizes(poolSizes)
													  .setMaxSets(_descriptorSetLayouts.size()));

	_descriptorSets = device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo()
														.setDescriptorPool(_descriptorPool)
														.setSetLayouts(_descriptorSetLayouts));
}

void vk_material::updateShaderDescriptors(const vk_shader& inShader)
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

	vk_renderer::get()->getDevice().updateDescriptorSets(writes, {});
}

std::map<std::string, std::vector<vk::DescriptorBufferInfo>> vk_material::getDescriptorBufferInfos(const vk_shader& inShader) const
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
							.setBuffer(buffer.getBuffer().get())
							.setOffset(buffer.getOffset())
							.setRange(buffer.getSize());
			descBufferInfo.first->second.push_back(info);
		}
	}

	return out;
}

std::map<std::string, std::vector<vk::DescriptorImageInfo>> vk_material::getDescriptorImageInfos(const vk_shader& inShader) const
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
			const auto image = bindIt != _images.end() ? bindIt->second[k] : &vk_renderer::get()->getTextureImagePlaceholder();
			auto info = vk::DescriptorImageInfo()
							.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
							.setImageView(image->getImageView())
							.setSampler(image->getSampler());
			descBufferInfo.first->second.push_back(info);
		}
	}
	return out;
}
