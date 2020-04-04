#pragma once
#include "vk_log.hxx"

#include <assert.h>
#include <vulkan/vulkan_core.h>

inline void vk_checkError(const VkResult& result)
{
	if (VK_SUCCESS != result)
	{
		std::cerr << "vk_checkError() assert: ";
		vk_logResult(result);

		assert(false);
	}
}