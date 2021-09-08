#pragma once
#include <vector>
#include <vulkan/vulkan.h>

class vk_descriptor_set final
{
public:
	vk_descriptor_set();
	vk_descriptor_set(const vk_descriptor_set&) = delete;
	vk_descriptor_set(vk_descriptor_set&&) = delete;
	~vk_descriptor_set();

	void create(const size_t descriptorSetsNum, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);

	void update(const std::vector<VkWriteDescriptorSet>& writeInfo);

	void destroy();

	const std::vector<VkDescriptorSet>& get() const;

protected:
	void createDescriptorPool(const VkDevice vkDevice, const size_t count);

	VkDescriptorSet createDescriptorSet(const VkDevice vkDevice, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);

private:
	VkDescriptorPool _vkDescriptorPool;

	std::vector<VkDescriptorSet> _vkDescriptorSets;
};