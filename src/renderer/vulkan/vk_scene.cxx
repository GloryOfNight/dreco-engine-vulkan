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

void vk_scene::create(const model& m)
{
	if (!isEmpty())
	{
		std::cerr << __FUNCTION__ << ": Scene already created! Destroy it before use again." << std::endl;
		return;
	}

	const size_t imagesNum{m._images.size()};
	_textureImages.resize(imagesNum, nullptr);
	for (size_t i = 0; i < imagesNum; ++i)
	{
		_textureImages[i] = new vk_texture_image();
		engine::get()->getThreadPool().queueTask(new async_load_texture_task(m._rootPath + '/' + m._images[i]._uri, this, i));
	}

	_graphicsPipelines.reserve(m._materials.size());
	for (const auto& mat : m._materials)
	{
		_graphicsPipelines.push_back(new vk_graphics_pipeline());
		_graphicsPipelines.back()->create(mat);
	}

	_meshes.reserve(m._meshes.size());
	for (const auto& scene : m._scenes)
	{
		for (const auto nodeIndex : scene._nodes)
		{
			recurseSceneNodes(m, m._nodes[nodeIndex], mat4::makeIdentity());
		}
	}
}

void vk_scene::recurseSceneNodes(const model& m, const node& selfNode, const mat4& rootMat)
{
	const mat4 newRootMat = selfNode._matrix * rootMat;
	if (selfNode._mesh != UINT32_MAX)
	{
		const auto& mesh = m._meshes[selfNode._mesh];
		_meshes.push_back(new vk_mesh());
		_meshes.back()->create(mesh, this);
		_meshes.back()->_mat = newRootMat;
	}
	for (const auto& childNodeIndex : selfNode._children)
	{
		recurseSceneNodes(m, m._nodes[childNodeIndex], newRootMat);
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