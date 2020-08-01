#pragma once
#include "vk_queue_family.hxx"
#include "vk_surface.hxx"
#include "vk_physical_device.hxx"
#include "vk_device.hxx"

#include "math/vec3.hxx"
#include "renderer/containers/mesh_data.hxx"
#include "renderer/containers/uniform_buffer.hxx"

#include <vulkan/vulkan.h>
#include <SDL.h>
#include <vector>

class engine;

class vk_renderer
{
public:
	vk_renderer(engine* eng);
	~vk_renderer();

	void tick(const float& delta_time);

	SDL_Window* getWindow() const;

protected:
	void drawFrame();

	inline void createWindow();

	inline void createInstance();

	inline void createSwapchain();

	inline void createImageViews();

	inline void createRenderPass();

	inline void createFramebuffers();

	inline void createCommandPool();

	inline void createCommandBuffers();

	inline void createPipelineLayout();

	inline void createGraphicsPipeline();

	inline void createShaderModule(const char* src, const size_t& src_size, VkShaderModule& shaderModule);

	inline void recordCommandBuffers();

	inline void createSemaphores();

	inline void cleanupSwapchain(VkSwapchainKHR& swapchain);

	inline void recreateSwapchain();

	void createDescriptorPool();

	void createDescriptorSets();

	void updateUniformBuffers(uint32_t image);

	// should be called before createPipelineLayout()
	void createDescriptorSetLayout();

	void createVertexBuffer();

	void createIndexBuffer();
	
	void createUniformBuffers();

	void createBuffer(VkDeviceSize size, VkBuffer& buffer, VkBufferUsageFlags bufferUsageFlags,
		VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags memoryPropertyFlags);

	void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);

	void destroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertiesFlags);

private:

	mesh_data mesh = mesh_data::createSprite();
	uniform_buffer ubo{};

	engine* _engine;

	SDL_Window* window;

	vk_surface surface;

	vk_physical_device physical_device;

	vk_queue_family queueFamily;

	vk_device device;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkAllocationCallbacks* mAllocator;

	VkInstance mInstance;

	VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
	std::vector<VkImageView> mSwapchainImageViews;
	std::vector<VkFramebuffer> mSwapchainFramebuffers;

	VkRenderPass mRenderPass;

	VkDescriptorPool mDescriptorPool;
	std::vector<VkDescriptorSet> mDescriptorSets;

	VkCommandPool mGraphicsCommandPool;
	VkCommandPool mTransferCommandPool;

	std::vector<VkCommandBuffer> mGraphicsCommandBuffers;

	VkPipelineLayout mPipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;

	VkPipeline mPipeline;

	VkSemaphore mSepaphore_Image_Avaible;

	VkSemaphore mSepaphore_Render_Finished;
};
