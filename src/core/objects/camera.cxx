#include "camera.hxx"

#include "renderer/vulkan/vk_renderer.hxx"

mat4 camera::getView() const
{
	return _view;
}

mat4 camera::getProjection() const
{
	return _projection;
}

void camera::tick(double deltaTime)
{
	world_object::tick(deltaTime);

	{ // update view
		transform viewTransform = getTransform();
		viewTransform._translation._x = -viewTransform._translation._x;
		viewTransform._translation._y = -viewTransform._translation._y;
		viewTransform._translation._z = -viewTransform._translation._z;
		viewTransform._rotation._pitch = -viewTransform._rotation._pitch;
		viewTransform._scale._x = -viewTransform._scale._x;

		_view = mat4::makeTransform(viewTransform);
	}

	{ // update projection
		const vk::Extent2D currentExtent = vk_renderer::get()->getCurrentExtent();
		_projection = mat4::makeProjection(0.1F, farZ, static_cast<float>(currentExtent.width) / static_cast<float>(currentExtent.height), 45.F);
	}
}
