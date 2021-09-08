#include "vk_descriptor_set.hxx"

#include "vk_allocator.hxx"
#include "vk_renderer.hxx"
#include "vk_utils.hxx"

vk_descriptor_set::vk_descriptor_set()
	: _vkDescriptorPool{VK_NULL_HANDLE}
{
}

vk_descriptor_set::~vk_descriptor_set()
{
}

void vk_descriptor_set::create(const size_t descriptorSetsNum, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{
	VkDevice vkDevice = vk_renderer::get()->getDevice().get();
	createDescriptorPool(vkDevice, descriptorSetsNum);

	_vkDescriptorSets.resize(descriptorSetsNum);
	for (size_t i = 0; i < descriptorSetsNum; ++i)
	{
		_vkDescriptorSets[i] = createDescriptorSet(vkDevice, descriptorSetLayouts);
	}
}

void vk_descriptor_set::update(const std::vector<VkWriteDescriptorSet>& writeInfo)
{
	VkDevice vkDevice = vk_renderer::get()->getDevice().get();
	vkUpdateDescriptorSets(vkDevice, writeInfo.size(), writeInfo.data(), 0, VK_NULL_HANDLE);
}

void vk_descriptor_set::destroy()
{
	if (VK_NULL_HANDLE != _vkDescriptorPool)
	{
		VkDevice vkDevice = vk_renderer::get()->getDevice().get();
		vkDestroyDescriptorPool(vkDevice, _vkDescriptorPool, vkGetAllocator());
		_vkDescriptorPool = VK_NULL_HANDLE;
	}
}

const std::vector<VkDescriptorSet>& vk_descriptor_set::get() const
{
	return _vkDescriptorSets;
}

void vk_descriptor_set::createDescriptorPool(const VkDevice vkDevice, const size_t count)
{
	VkDescriptorPoolSize uniformSize{};
	uniformSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformSize.descriptorCount = count;

	VkDescriptorPoolSize sampledImageSize{};
	sampledImageSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampledImageSize.descriptorCount = count;

	std::vector<VkDescriptorPoolSize> poolSizes{uniformSize, sampledImageSize};

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.pNext = nullptr;
	poolCreateInfo.flags = 0;
	poolCreateInfo.poolSizeCount = poolSizes.size();
	poolCreateInfo.pPoolSizes = poolSizes.data();
	poolCreateInfo.maxSets = count;

	VK_CHECK(vkCreateDescriptorPool(vkDevice, &poolCreateInfo, VK_NULL_HANDLE, &_vkDescriptorPool));
}

VkDescriptorSet vk_descriptor_set::createDescriptorSet(const VkDevice vkDevice, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = _vkDescriptorPool;
	allocInfo.descriptorSetCount = descriptorSetLayouts.size();
	allocInfo.pSetLayouts = descriptorSetLayouts.data();

	VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
	VK_CHECK(vkAllocateDescriptorSets(vkDevice, &allocInfo, &descriptorSet));
	return descriptorSet;
}
