#pragma once
#include "core/threads/thread_pool.hxx"
#include "core/containers/image_data.hxx"
#include <string_view>

struct async_load_image : public thread_task
{
	async_load_texture_task(const std::string_view imageUri)
		: _imageUri(imageUri)
		, _image{}
	{
	}

	virtual void doJob() override
	{
		_image.load(_imageUri);
	};

	image_data extract() { return std::move(_image); };

private:
	std::string _imageUri;

	image_data _image;
};