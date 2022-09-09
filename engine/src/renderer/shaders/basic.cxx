#include "basic.hxx"

#include "renderer/containers/camera_data.hxx"
#include "renderer/vk_mesh.hxx"
#include "renderer/vk_renderer.hxx"

#include "dreco.hxx"

vk_shader_basic_vert::vk_shader_basic_vert()
	: vk_shader()
{
	_shaderPath = DRECO_SHADER("basic.vert.spv");
}

void vk_shader_basic_vert::addDescriptorWriteInfos(vk_descriptor_write_infos& infos, const vk_graphics_pipeline& pipeline) const
{
	const auto& buffer = vk_renderer::get()->getCameraDataBuffer();

	infos.bufferInfos.push_back(vk::DescriptorBufferInfo()
									.setBuffer(buffer.get())
									.setOffset(0)
									.setRange(buffer.getSize()));
}

void vk_shader_basic_vert::cmdPushConstants(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, const vk_mesh* mesh) const
{
	commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(mat4), &mesh->_mat);
}

vk_shader_basic_frag::vk_shader_basic_frag()
	: vk_shader()
{
	_shaderPath = DRECO_SHADER("basic.frag.spv");
}

void vk_shader_basic_frag::addDescriptorWriteInfos(vk_descriptor_write_infos& infos, const vk_graphics_pipeline& pipeline) const
{
	const auto& material = pipeline.getMaterial();
	const auto& materialBuffer = pipeline.getMaterialBuffer();

	const auto& baseColor = pipeline.getTextureImageFromIndex(material._baseColorIndex);
	const auto& metallicRoughness = pipeline.getTextureImageFromIndex(material._metallicRoughnessIndex);
	const auto& normal = pipeline.getTextureImageFromIndex(material._normalIndex);
	const auto& emissive = pipeline.getTextureImageFromIndex(material._emissiveIndex);

	infos.bufferInfos.push_back(vk::DescriptorBufferInfo()
									.setBuffer(materialBuffer.get())
									.setOffset(0)
									.setRange(materialBuffer.getSize()));

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
								   .setImageView(emissive.getImageView())
								   .setSampler(emissive.getSampler()));
}
