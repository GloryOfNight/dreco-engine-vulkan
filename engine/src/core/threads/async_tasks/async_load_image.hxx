#pragma once
#include "core/threads/thread_pool.hxx"
#include "core/containers/image_data.hxx"
#include <string_view>

struct async_load_image : public thread_task
{
	template<typename Str>
	async_load_texture_task(Str&& imageUri)
		: _imageUri(imageUri)
		, _image{}
	{
	}

	virtual void init() override{};

	virtual void doJob() override
	{
		_texData.load(_texUri);
	};

	virtual void completed() override{};

private:
	std::string _imageUri;

	image_data _image;
};