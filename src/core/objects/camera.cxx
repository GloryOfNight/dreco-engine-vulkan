#include "camera.hxx"
#include "renderer/vulkan/vk_renderer.hxx"

mat4 camera::getView() const
{
	transform viewTransform = getTransform();
	viewTransform._translation._x = -viewTransform._translation._x;
	viewTransform._translation._y = -viewTransform._translation._y;
	viewTransform._translation._z = -viewTransform._translation._z;

	viewTransform._rotation._pitch = -viewTransform._rotation._pitch;

	viewTransform._scale._x = -viewTransform._scale._x;

	return mat4::makeTransform(viewTransform);
}

mat4 camera::getProjection() const
{
	const vk_renderer* renderer = vk_renderer::get();
	const auto surfaceCapabilities = renderer->getPhysicalDevice().getSurfaceCapabilitiesKHR(renderer->getSurface());
	const vk::Extent2D currentExtent = surfaceCapabilities.currentExtent;

	return mat4::makeProjection(0.1F, 1000, static_cast<float>(currentExtent.width) / static_cast<float>(currentExtent.height), 45.F);
}
