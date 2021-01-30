#include "vk_descriptor_set.hxx"

#include "vk_allocator.hxx"
#include "vk_renderer.hxx"
#include "vk_utils.hxx"

vk_descriptor_set::vk_descriptor_set()
	: _vkDescriptorPool{VK_NULL_HANDLE}
	, _vkDescriptorSetLayouts{VK_NULL_HANDLE}
	, _vkDescriptorSet{VK_NULL_HANDLE}
{
}

vk_descriptor_set::~vk_descriptor_set()
{
}

void vk_descriptor_set::create()
{
	VkDevice vkDevice = vk_renderer::get()->getDevice().get();
	createDescriptorPool(vkDevice);
	createDescriptorSetLayout(vkDevice);
	createDescriptorSet(vkDevice);
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
		for (VkDescriptorSetLayout& layout : _vkDescriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(vkDevice, layout, vkGetAllocator());
		}
		_vkDescriptorSetLayouts.clear();

		vkDestroyDescriptorPool(vkDevice, _vkDescriptorPool, vkGetAllocator());
		_vkDescriptorPool = VK_NULL_HANDLE;
	}
}

VkDescriptorSet vk_descriptor_set::get() const
{
	return _vkDescriptorSet;
}

const std::vector<VkDescriptorSetLayout>& vk_descriptor_set::getLayouts() const
{
	return _vkDescriptorSetLayouts;
}

void vk_descriptor_set::createDescriptorPool(VkDevice vkDevice)
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.pNext = nullptr;
	poolCreateInfo.flags = 0;
	poolCreateInfo.poolSizeCount = 1;
	poolCreateInfo.pPoolSizes = &poolSize;
	poolCreateInfo.maxSets = 1;

	VK_CHECK(vkCreateDescriptorPool(vkDevice, &poolCreateInfo, VK_NULL_HANDLE, &_vkDescriptorPool));
}

void vk_descriptor_set::createDescriptorSetLayout(VkDevice vkDevice)
{
	_vkDescriptorSetLayouts.resize(1);

	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = VK_NULL_HANDLE;

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pNext = nullptr;
	layoutCreateInfo.flags = 0;
	layoutCreateInfo.bindingCount = 1;
	layoutCreateInfo.pBindings = &layoutBinding;

	VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &layoutCreateInfo, VK_NULL_HANDLE, _vkDescriptorSetLayouts.data()));
}

void vk_descriptor_set::createDescriptorSet(VkDevice vkDevice)
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = _vkDescriptorPool;
	allocInfo.descriptorSetCount = _vkDescriptorSetLayouts.size();
	allocInfo.pSetLayouts = _vkDescriptorSetLayouts.data();

	VK_CHECK(vkAllocateDescriptorSets(vkDevice, &allocInfo, &_vkDescriptorSet));
}
