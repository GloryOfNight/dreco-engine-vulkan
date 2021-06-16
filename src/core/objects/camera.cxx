#include "camera.hxx"
#include "renderer/vulkan/vk_renderer.hxx"

mat4 camera::getView() const
{
	transform viewTransform = getTransform();
	viewTransform._translation._x = -viewTransform._translation._x;
	viewTransform._translation._y = -viewTransform._translation._y;
	viewTransform._translation._z = -viewTransform._translation._z;

	viewTransform._rotation._pitch = -viewTransform._rotation._pitch;

	return mat4::makeTransform(viewTransform);
}

mat4 camera::getProjection() const
{
	const VkExtent2D vkCurrentExtent = vk_renderer::get()->getSurface().getCapabilities().currentExtent;
	return mat4::makeProjection(0.1F, 1000, static_cast<float>(vkCurrentExtent.width) / static_cast<float>(vkCurrentExtent.height), 45.F);
}
