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
	VkDescriptorPoolSize uniformSize{};
	uniformSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformSize.descriptorCount = 1;

	VkDescriptorPoolSize sampledImageSize{};
	sampledImageSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampledImageSize.descriptorCount = 1;

	std::vector<VkDescriptorPoolSize> poolSizes{uniformSize, sampledImageSize};

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.pNext = nullptr;
	poolCreateInfo.flags = 0;
	poolCreateInfo.poolSizeCount = poolSizes.size();
	poolCreateInfo.pPoolSizes = poolSizes.data();
	poolCreateInfo.maxSets = 1;

	VK_CHECK(vkCreateDescriptorPool(vkDevice, &poolCreateInfo, VK_NULL_HANDLE, &_vkDescriptorPool));
}

void vk_descriptor_set::createDescriptorSetLayout(VkDevice vkDevice)
{
	VkDescriptorSetLayoutBinding uniformBinding{};
	uniformBinding.binding = 0;
	uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBinding.descriptorCount = 1;
	uniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uniformBinding.pImmutableSamplers = VK_NULL_HANDLE;

	VkDescriptorSetLayoutBinding sampledImageBinding{};
	sampledImageBinding.binding = 1;
	sampledImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampledImageBinding.descriptorCount = 1;
	sampledImageBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	sampledImageBinding.pImmutableSamplers = VK_NULL_HANDLE;

	std::vector<VkDescriptorSetLayoutBinding> layoutBindings{uniformBinding, sampledImageBinding};

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pNext = nullptr;
	layoutCreateInfo.flags = 0;
	layoutCreateInfo.bindingCount = layoutBindings.size();
	layoutCreateInfo.pBindings = layoutBindings.data();

	_vkDescriptorSetLayouts.resize(1);
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
