#pragma once
#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vulkan/vulkan_core.h>

inline const char* vk_resultToChar(VkResult& result);

#define GET_TYPE_NAME(Type) #Type

#define VK_CHECK(op)                                                                                 \
	{                                                                                                \
		const VkResult vkResult = op;                                                                \
		if (VK_SUCCESS != vkResult)                                                                  \
		{                                                                                            \
			const std::string error_message = "VK_CHECK: " + std::string(vk_resultToChar(vkResult)); \
			std::cerr << error_message;                                                              \
			throw std::runtime_error(error_message);                                                 \
		}                                                                                            \
	}

#define VK_RETURN_ON_RESULT(op, value)                                                                          \
	{                                                                                                           \
		const VkResult vkResult = op;                                                                           \
		if (value == vkResult)                                                                                  \
		{                                                                                                       \
			const std::string error_message = "VK_RETURN_ON_RESULT: " + std::string(vk_resultToChar(vkResult)); \
			std::cerr << error_message;                                                                         \
			return;                                                                                             \
		}                                                                                                       \
	}

inline const char* vk_resultToChar(const VkResult& result)
{
	const char* code;
	switch (result)
	{
	case VK_SUCCESS:
		code = GET_TYPE_NAME(VK_SUCCESS);
		break;
	case VK_NOT_READY:
		code = GET_TYPE_NAME(VK_NOT_READY);
		break;
	case VK_TIMEOUT:
		code = GET_TYPE_NAME(VK_TIMEOUT);
		break;
	case VK_EVENT_SET:
		code = GET_TYPE_NAME(VK_EVENT_SET);
		break;
	case VK_EVENT_RESET:
		code = GET_TYPE_NAME(VK_EVENT_RESET);
		break;
	case VK_INCOMPLETE:
		code = GET_TYPE_NAME(VK_INCOMPLETE);
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		code = GET_TYPE_NAME(VK_ERROR_OUT_OF_HOST_MEMORY);
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		code = GET_TYPE_NAME(VK_ERROR_OUT_OF_DEVICE_MEMORY);
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		code = GET_TYPE_NAME(VK_ERROR_INITIALIZATION_FAILED);
		break;
	case VK_ERROR_DEVICE_LOST:
		code = GET_TYPE_NAME(VK_ERROR_DEVICE_LOST);
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		code = GET_TYPE_NAME(VK_ERROR_MEMORY_MAP_FAILED);
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		code = GET_TYPE_NAME(VK_ERROR_LAYER_NOT_PRESENT);
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		code = GET_TYPE_NAME(VK_ERROR_EXTENSION_NOT_PRESENT);
		break;
	case VK_ERROR_FEATURE_NOT_PRESENT:
		code = GET_TYPE_NAME(VK_ERROR_FEATURE_NOT_PRESENT);
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		code = GET_TYPE_NAME(VK_ERROR_INCOMPATIBLE_DRIVER);
		break;
	case VK_ERROR_TOO_MANY_OBJECTS:
		code = GET_TYPE_NAME(VK_ERROR_TOO_MANY_OBJECTS);
		break;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		code = GET_TYPE_NAME(VK_ERROR_FORMAT_NOT_SUPPORTED);
		break;
	case VK_ERROR_FRAGMENTED_POOL:
		code = GET_TYPE_NAME(VK_ERROR_FRAGMENTED_POOL);
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		code = GET_TYPE_NAME(VK_ERROR_OUT_OF_DATE_KHR);
		break;
	default:
		code = "UNIPLEMENTED";
		break;
	}
	return code;
}