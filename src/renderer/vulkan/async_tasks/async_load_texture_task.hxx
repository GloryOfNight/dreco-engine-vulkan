#pragma once
#include "core/threads/thread_pool.hxx"
#include "renderer/vulkan/vk_mesh.hxx"
#include "renderer/vulkan/vk_renderer.hxx"
#include "renderer/vulkan/vk_scene.hxx"
#include "renderer/vulkan/vk_texture_image.hxx"

#include <string_view>

struct async_load_texture_task : public thread_task
{
	async_load_texture_task(const std::string_view& texUri, vk_scene* scene, const uint32_t texIndex)
		: _texUri{texUri}
		, _texIndex{texIndex}
		, _scene{scene}
		, _texData{nullptr}
	{
	}

	virtual void init() override{};

	virtual void doJob() override
	{
		_texData = texture_data::createNew(_texUri);
	};

	virtual void compeleted() override
	{
		vk_texture_image* texImage = _scene->getTextureImages()[_texIndex];
		new (texImage) vk_texture_image();
		texImage->create(*_texData);
		delete _texData;

		const auto& meshes = _scene->getMeshes();
		for (auto& mesh : meshes)
		{
			mesh->getDescriptorSet().rewrite({_texIndex, texImage});
		}
	};

private:
	std::string _texUri;

	uint32_t _texIndex;

	vk_scene* _scene;

	texture_data* _texData;
};