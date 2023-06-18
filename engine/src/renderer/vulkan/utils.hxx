#pragma once
#include "dreco.hxx"

#include <stdexcept>
#include <string>
#include <vulkan/vulkan.hpp>

#define GET_TYPE_NAME(Type) #Type

#define VK_CHECK(op)                                                              \
	{                                                                             \
		const vk::Result vkResult = static_cast<vk::Result>(op);                  \
		if (vk::Result::eSuccess != vkResult)                                     \
		{                                                                         \
			const std::string error_message = "VK_CHECK: " + to_string(vkResult); \
			DE_LOG(Critical, error_message.data());                               \
			throw std::runtime_error(error_message.data());                       \
		}                                                                         \
	}

#define VK_RETURN_ON_RESULT(op, value)                                                       \
	{                                                                                        \
		const vk::Result vkResult = static_cast<vk::Result>(op);                             \
		if (static_cast<vk::Result>(value) == vkResult)                                      \
		{                                                                                    \
			const std::string error_message = "VK_RETURN_ON_RESULT: " + to_string(vkResult); \
			DE_LOG(Critical, error_message.data());                                          \
			return;                                                                          \
		}                                                                                    \
	}

namespace de::vulkan
{
	struct utils
	{
		struct memory_property
		{
			constexpr static inline vk::MemoryPropertyFlags host = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			constexpr static inline vk::MemoryPropertyFlags device = vk::MemoryPropertyFlagBits::eDeviceLocal;
		};

		struct find_supported_format_info
		{
			std::vector<vk::Format> _formatCandidates;
			vk::ImageTiling _imageTiling;
			vk::FormatFeatureFlags _formatFeatureFlags;
		};
		static vk::Format findSupportedDepthFormat(const vk::PhysicalDevice physicalDevice) noexcept;

		static vk::SampleCountFlagBits findMaxSampleCount(const vk::PhysicalDevice physicalDevice);

		static vk::SurfaceFormatKHR findSurfaceFormat(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface, const vk::Format format);

		static vk::PresentModeKHR findPresentMode(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface);
	};
} // namespace de::vulkan