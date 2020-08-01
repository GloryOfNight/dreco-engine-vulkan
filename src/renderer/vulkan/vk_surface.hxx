#pragma once
#include <vulkan/vulkan_core.h>

struct SDL_Window;

class vk_surface
{
public:
	vk_surface(const VkInstance* vkInstance);
	~vk_surface();

	void create(SDL_Window* window);

	void setup(VkPhysicalDevice vkPhysicalDevice);	

	void destroy();

	VkSurfaceKHR get() const;

	const VkSurfaceCapabilitiesKHR& getCapabilities() const;

	const VkSurfaceFormatKHR& getFormat() const;

	VkPresentModeKHR getPresentMode() const;

protected:

	inline void setupSurfaceFormats(VkPhysicalDevice vkPhysicaLDevice);
	
	inline void setupPresentModes(VkPhysicalDevice vkPhysicaLDevice);

	inline void setupSurfaceCapabilities(VkPhysicalDevice vkPhysicaLDevice);

private:

	const VkInstance* _vkInstance;

	VkSurfaceKHR _vkSurface;

	VkSurfaceCapabilitiesKHR _vkSurfaceCapabilities;

	VkSurfaceFormatKHR _vkSurfaceFormat;

	VkPresentModeKHR _vkPresentMode;
};