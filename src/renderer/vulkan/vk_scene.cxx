#include "vk_scene.hxx"

#include "async_tasks/async_load_texture_task.hxx"
#include "core/utils/utils.hxx"
#include "engine/engine.hxx"

#include "vk_graphics_pipeline.hxx"
#include "vk_mesh.hxx"
#include "vk_texture_image.hxx"

#include <iostream>

vk_scene::~vk_scene()
{
	if (!isEmpty())
	{
		destroy();
	}
}

void vk_scene::create(const scene& scn)
{
	if (!isEmpty())
	{
		std::cerr << __FUNCTION__ << ": Scene already created! Destroy it before use again." << std::endl;
		return;
	}

	const size_t imagesNum{scn._images.size()};
	_textureImages.resize(imagesNum, nullptr);
	for (size_t i = 0; i < imagesNum; ++i)
	{
		_textureImages[i] = new vk_texture_image();
		engine::get()->getThreadPool()->queueTask(new async_load_texture_task(scn._images[i]._uri, this, i));
	}

	_graphicsPipelines.reserve(scn._materials.size());
	for (const auto& mat : scn._materials)
	{
		_graphicsPipelines.push_back(new vk_graphics_pipeline());
		_graphicsPipelines.back()->create(mat);
	}

	_meshes.reserve(scn._meshes.size());
	for (auto& mesh : scn._meshes)
	{
		_meshes.push_back(new vk_mesh());
		_meshes.back()->create(mesh, this);
	}
}

void vk_scene::update()
{
	for (auto* mesh : _meshes)
	{
		mesh->update();
	}
}

void vk_scene::recreatePipelines()
{
	for (auto* pipeline : _graphicsPipelines)
	{
		pipeline->recreatePipeline();
	}
}

void vk_scene::bindToCmdBuffer(VkCommandBuffer commandBuffer)
{
	for (auto* pipeline : _graphicsPipelines)
	{
		pipeline->bindToCmdBuffer(commandBuffer);
	}
	for (auto* mesh : _meshes)
	{
		mesh->bindToCmdBuffer(commandBuffer);
	}
}

bool vk_scene::isEmpty() const
{
	return _textureImages.empty() && _graphicsPipelines.empty() && _meshes.empty();
}

void vk_scene::destroy()
{
	clearVectorOfPtr(_textureImages);
	clearVectorOfPtr(_graphicsPipelines);
	clearVectorOfPtr(_meshes);
}