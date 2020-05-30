#pragma once
#include <vulkan/vulkan_core.h>

#include <iostream>

inline void vk_logResult(const VkResult& result)
{
	const char* code;
	switch (result)
	{
		case VK_SUCCESS:
			code = "VK_SUCCESS";
			break;
		case VK_NOT_READY:
			code = "VK_NOT_READY";
			break;
		case VK_TIMEOUT:
			code = "VK_TIMEOUT";
			break;
		case VK_EVENT_SET:
			code = "VK_EVENT_SET";
			break;
		case VK_EVENT_RESET:
			code = "VK_EVENT_RESET";
			break;
		case VK_INCOMPLETE:
			code = "VK_INCOMPLETE";
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			code = "VK_INCOMPLETE";
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			code = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			code = "VK_ERROR_INITIALIZATION_FAILED";
			break;
		case VK_ERROR_DEVICE_LOST:
			code = "VK_ERROR_DEVICE_LOST";
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			code = "VK_ERROR_MEMORY_MAP_FAILED";
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			code = "VK_ERROR_LAYER_NOT_PRESENT";
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			code = "VK_ERROR_EXTENSION_NOT_PRESENT";
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			code = "VK_ERROR_FEATURE_NOT_PRESENT";
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			code = "VK_ERROR_INCOMPATIBLE_DRIVER";
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			code = "VK_ERROR_TOO_MANY_OBJECTS";
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			code = "VK_ERROR_FORMAT_NOT_SUPPORTED";
			break;
		case VK_ERROR_FRAGMENTED_POOL:
			code = "VK_ERROR_FRAGMENTED_POOL";
			break;
		default:
			code = "UNIPLEMENTED";
			break;
	}
	std::cout << "VkResult: " << code << std::endl;
}