#include "vk_surface.hxx"

#include "vk_allocator.hxx"
#include "vk_utils.hxx"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vector>

vk_surface::vk_surface()
	: _vkSurface{VK_NULL_HANDLE}
	, _vkSurfaceCapabilities{}
	, _vkSurfaceFormat{}
	, _vkPresentMode{}
{
}

void vk_surface::create(const VkInstance vkInstance, SDL_Window* window)
{
	if (SDL_Vulkan_CreateSurface(window, vkInstance, &_vkSurface) != SDL_TRUE)
	{
		throw std::runtime_error(std::string("Failed create surface with error: ") + SDL_GetError());
	}
}

void vk_surface::setup(VkPhysicalDevice vkPhysicalDevice)
{
	setupSurfaceFormats(vkPhysicalDevice);
	setupPresentModes(vkPhysicalDevice);
	setupSurfaceCapabilities(vkPhysicalDevice);
}

void vk_surface::destroy(const VkInstance vkInstance)
{
	if (VK_NULL_HANDLE != _vkSurface)
	{
		vkDestroySurfaceKHR(vkInstance, _vkSurface, vkGetAllocator());
		_vkSurface = VK_NULL_HANDLE;
	}
}

VkSurfaceKHR vk_surface::get() const
{
	return _vkSurface;
}

const VkSurfaceCapabilitiesKHR& vk_surface::getCapabilities() const
{
	return _vkSurfaceCapabilities;
}

const VkSurfaceFormatKHR& vk_surface::getFormat() const
{
	return _vkSurfaceFormat;
}

VkPresentModeKHR vk_surface::getPresentMode() const
{
	return _vkPresentMode;
}

void vk_surface::setupSurfaceFormats(VkPhysicalDevice vkPhysicaLDevice)
{
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicaLDevice, _vkSurface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> vkSurfaceFormats(formatCount);
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicaLDevice, _vkSurface, &formatCount, vkSurfaceFormats.data()));

	for (const auto& vkSurfaceFormat : vkSurfaceFormats)
	{
		_vkSurfaceFormat = vkSurfaceFormat;
		if (VK_FORMAT_B8G8R8A8_UNORM == vkSurfaceFormat.format)
		{
			break;
		}
	}
}

void vk_surface::setupPresentModes(VkPhysicalDevice vkPhysicaLDevice)
{
	uint32_t presentCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicaLDevice, _vkSurface, &presentCount, nullptr);
	std::vector<VkPresentModeKHR> vkPresentModes(presentCount, {});
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicaLDevice, _vkSurface, &presentCount, vkPresentModes.data()));

	for (const auto& vkPresentMode : vkPresentModes)
	{
		_vkPresentMode = vkPresentMode;
		if (VK_PRESENT_MODE_MAILBOX_KHR == vkPresentMode)
		{
			break;
		}
	}
}

inline void vk_surface::setupSurfaceCapabilities(VkPhysicalDevice vkPhysicaLDevice)
{
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicaLDevice, _vkSurface, &_vkSurfaceCapabilities));
}
