#pragma once

#include "images/texture_image.hxx"

#include "buffer.hxx"
#include "shader.hxx"

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class material;
	class material_instance final
	{
		friend material;
		material_instance(material* owner);

	public:
		using unique = std::unique_ptr<material_instance>;

		material_instance(material_instance&) = delete;
		material_instance(material_instance&&) = default;
		~material_instance() = default;

		material* getMaterial() const;

		template <typename Str>
		void setBufferDependency(Str&& inName, const de::vulkan::buffer* inBuffer, size_t arrayIndex = 0);

		template <typename Str>
		void setBufferDependencySize(Str&& inName, size_t size);

		template <typename Str>
		void setImageDependecy(Str&& inName, const texture_image* inImage, size_t arrayIndex = 0);

		template <typename Str>
		void setImageDependecySize(Str&& inName, size_t size);

		void updateDescriptorSets();

		void bindCmd(vk::CommandBuffer commandBuffer) const;

	private:
		void updateShaderDescriptors(const shader& inShader);
		std::map<std::string, std::vector<vk::DescriptorBufferInfo>> getDescriptorBufferInfos(const shader& inShader) const;
		std::map<std::string, std::vector<vk::DescriptorImageInfo>> getDescriptorImageInfos(const shader& inShader) const;

		material* _owner;

		std::vector<vk::DescriptorSet> _descriptorSets;

		std::map<std::string, std::vector<const de::vulkan::buffer*>> _buffers;
		std::map<std::string, std::vector<const texture_image*>> _images;
	};

	template <typename Str>
	void material_instance::setBufferDependency(Str&& inName, const de::vulkan::buffer* inBuffer, size_t arrayIndex)
	{
		auto it = _buffers.try_emplace(std::forward<Str>(inName), std::vector<const de::vulkan::buffer*>(1, nullptr));
		it.first->second[arrayIndex] = inBuffer;
	}

	template <typename Str>
	void material_instance::setBufferDependencySize(Str&& inName, size_t size)
	{
		auto& arr = _buffers[std::forward<Str>(inName)];
		if (arr.size() != size)
		{
			arr.resize(size);
		}
	}

	template <typename Str>
	void material_instance::setImageDependecy(Str&& inName, const texture_image* inImage, size_t arrayIndex)
	{
		auto it = _images.try_emplace(std::forward<Str>(inName), std::vector<const texture_image*>(1, nullptr));
		it.first->second[arrayIndex] = inImage;
	}

	template <typename Str>
	void material_instance::setImageDependecySize(Str&& inName, size_t size)
	{
		auto& arr = _images[std::forward<Str>(inName)];
		if (arr.size() != size)
		{
			arr.resize(size);
		}
	}
} // namespace de::vulkan