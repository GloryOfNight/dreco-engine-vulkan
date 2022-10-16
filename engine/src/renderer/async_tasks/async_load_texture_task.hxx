#pragma once
#include "core/threads/thread_pool.hxx"
#include "core/utils/log.hxx"
#include "renderer/vk_mesh.hxx"
#include "renderer/vk_renderer.hxx"
#include "renderer/vk_scene.hxx"
#include "renderer/vk_texture_image.hxx"

#include <string_view>

struct async_load_texture_task : public thread_task
{
	async_load_texture_task(const std::string_view& texUri, vk_scene* scene, const uint32_t texIndex)
		: _texUri{texUri}
		, _texIndex{texIndex}
		, _scene{scene}
		, _texData{}
	{
	}

	virtual void init() override{};

	virtual void doJob() override
	{
		_texData.load(_texUri);
	};

	virtual void completed() override
	{
		auto& texImage = _scene->getTextureImages()[_texIndex];
		new (texImage.get()) vk_texture_image();

		if (!_texData.isLoaded())
		{
			DE_LOG(Error, "Failed to load texture from uri: %s", _texUri.data());
		}
		texImage->create(_texData);

		//const auto& pipelines = _scene->getGraphicPipelines();
		//for (auto& pipeline : pipelines)
		//{
		//	pipeline->updateDescriptiors();
		//}
	};

private:
	std::string _texUri;

	uint32_t _texIndex;

	vk_scene* _scene;

	image_data _texData;
};