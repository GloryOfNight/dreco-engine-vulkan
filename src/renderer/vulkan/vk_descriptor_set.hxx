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

	void create();

	void update(const std::vector<VkWriteDescriptorSet>& writeInfo);

	void destroy();

	VkDescriptorSet get() const;

	const std::vector<VkDescriptorSetLayout>& getLayouts() const;

protected:
	void createDescriptorPool(VkDevice vkDevice);

	void createDescriptorSetLayout(VkDevice vkDevice);

	void createDescriptorSet(VkDevice vkDevice);

private:
	VkDescriptorPool _vkDescriptorPool;

	std::vector<VkDescriptorSetLayout> _vkDescriptorSetLayouts;

	VkDescriptorSet _vkDescriptorSet;
};