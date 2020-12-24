#pragma once
#include <vulkan/vulkan.h>

struct SDL_Window;

class vk_surface
{
public:
	vk_surface();
	vk_surface(const vk_surface&) = delete;
	vk_surface(vk_surface&&) = delete;
	~vk_surface() = default;

	vk_surface& operator=(const vk_surface&) = delete;
	vk_surface& operator=(vk_surface&&) = delete;

	void create(const VkInstance vkInstance, SDL_Window* window);

	void setup(VkPhysicalDevice vkPhysicalDevice);

	void destroy(const VkInstance vkInstance);

	VkSurfaceKHR get() const;

	const VkSurfaceCapabilitiesKHR& getCapabilities() const;

	const VkSurfaceFormatKHR& getFormat() const;

	VkPresentModeKHR getPresentMode() const;

protected:
	inline void setupSurfaceFormats(VkPhysicalDevice vkPhysicaLDevice);

	inline void setupPresentModes(VkPhysicalDevice vkPhysicaLDevice);

	inline void setupSurfaceCapabilities(VkPhysicalDevice vkPhysicaLDevice);

private:
	VkSurfaceKHR _vkSurface;

	VkSurfaceCapabilitiesKHR _vkSurfaceCapabilities;

	VkSurfaceFormatKHR _vkSurfaceFormat;

	VkPresentModeKHR _vkPresentMode;
};