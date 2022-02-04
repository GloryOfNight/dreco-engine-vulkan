#include "basic.hxx"

#include "renderer/containers/camera_data.hxx"
#include "renderer/vulkan/vk_mesh.hxx"
#include "renderer/vulkan/vk_renderer.hxx"

#include "dreco.hxx"

vk_shader_basic_vert::vk_shader_basic_vert()
	: vk_shader()
{
	_shaderPath = DRECO_SHADER("basic.vert.spv");
}

void vk_shader_basic_vert::addPipelineShaderStageCreateInfo(std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages) const
{
	shaderStages.push_back(vk::PipelineShaderStageCreateInfo()
							   .setModule(_shaderModule)
							   .setStage(vk::ShaderStageFlagBits::eVertex)
							   .setPName("main"));
}

void vk_shader_basic_vert::addDescriptorSetLayoutBindings(std::vector<vk::DescriptorSetLayoutBinding>& bindings) const
{
	bindings.push_back(vk::DescriptorSetLayoutBinding()
						   .setBinding(0)
						   .setDescriptorType(vk::DescriptorType::eUniformBuffer)
						   .setDescriptorCount(1)
						   .setStageFlags(vk::ShaderStageFlagBits::eVertex));
}

void vk_shader_basic_vert::addPushConstantRange(std::vector<vk::PushConstantRange>& ranges) const
{
	ranges.push_back(vk::PushConstantRange().setStageFlags(vk::ShaderStageFlagBits::eVertex).setSize(sizeof(camera_data)));
}

void vk_shader_basic_vert::addDescriptorPoolSizes(std::vector<vk::DescriptorPoolSize>& sizes) const
{
	sizes.push_back(vk::DescriptorPoolSize().setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1));
}

void vk_shader_basic_vert::addDescriptorWriteInfos(vk_descriptor_write_infos& infos, const vk_scene* scene, const material& mat) const
{
	const auto& buffer = vk_renderer::get()->getCameraDataBuffer();

	auto& bufferInfo = infos.bufferInfos.emplace_back(vk::DescriptorBufferInfo());
	bufferInfo.buffer = buffer.get();
	bufferInfo.offset = 0;
	bufferInfo.range = buffer.getSize();
}

void vk_shader_basic_vert::cmdPushConstants(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, const vk_mesh* mesh)
{
	commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(mat4), &mesh->_mat);
}

vk_shader_basic_frag::vk_shader_basic_frag()
	: vk_shader()
{
	_shaderPath = DRECO_SHADER("basic.frag.spv");
}

void vk_shader_basic_frag::addPipelineShaderStageCreateInfo(std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages) const
{
	shaderStages.push_back(vk::PipelineShaderStageCreateInfo()
							   .setModule(_shaderModule)
							   .setStage(vk::ShaderStageFlagBits::eFragment)
							   .setPName("main"));
}

void vk_shader_basic_frag::addDescriptorSetLayoutBindings(std::vector<vk::DescriptorSetLayoutBinding>& bindings) const
{
	bindings.push_back(vk::DescriptorSetLayoutBinding()
						   .setBinding(1)
						   .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
						   .setDescriptorCount(4)
						   .setStageFlags(vk::ShaderStageFlagBits::eFragment));
}

void vk_shader_basic_frag::addDescriptorPoolSizes(std::vector<vk::DescriptorPoolSize>& sizes) const
{
	sizes.push_back(vk::DescriptorPoolSize().setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1));
}

void vk_shader_basic_frag::addDescriptorWriteInfos(vk_descriptor_write_infos& infos, const vk_scene* scene, const material& mat) const
{
	const auto& baseColor = scene->getTextureImageFromIndex(mat._pbrMetallicRoughness._baseColorTexture._index);
	const auto& metallicRoughness = scene->getTextureImageFromIndex(mat._pbrMetallicRoughness._metallicRoughnessTexture._index);
	const auto& normal = scene->getTextureImageFromIndex(mat._normalTexture._index);
	const auto& emmisive = scene->getTextureImageFromIndex(mat._emissiveTexture._index);

	infos.imageInfos.reserve(4);
	infos.imageInfos.push_back(vk::DescriptorImageInfo()
								   .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
								   .setImageView(baseColor.getImageView())
								   .setSampler(baseColor.getSampler()));

	infos.imageInfos.push_back(vk::DescriptorImageInfo()
								   .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
								   .setImageView(metallicRoughness.getImageView())
								   .setSampler(metallicRoughness.getSampler()));

	infos.imageInfos.push_back(vk::DescriptorImageInfo()
								   .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
								   .setImageView(normal.getImageView())
								   .setSampler(normal.getSampler()));

	infos.imageInfos.push_back(vk::DescriptorImageInfo()
								   .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
								   .setImageView(emmisive.getImageView())
								   .setSampler(emmisive.getSampler()));
}
