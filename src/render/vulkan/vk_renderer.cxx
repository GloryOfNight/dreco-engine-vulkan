#include "vk_renderer.hxx"
#include <iostream>

vk_renderer::vk_renderer(engine *eng) : _engine(eng), allocator(nullptr)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(540, 720, "engine", nullptr, nullptr);

    // clang-format off
    VkApplicationInfo appInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "dreco-test",
        .applicationVersion = 0,
        .pEngineName = "dreco-engine",
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_1
    };
    
    uint32_t count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    VkInstanceCreateInfo createInfo
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .enabledExtensionCount = count,
        .ppEnabledExtensionNames = extensions
    };
    // cland-format on

    if (VkResult result = vkCreateInstance(&createInfo, allocator, &instance); VK_SUCCESS != result)
    {
        std::cerr << "vkCreateInstance(): failed with result: " << result << std::endl;
    }
    if (GL_FALSE == glfwVulkanSupported()) 
    {
        std::cerr << "Vulkan not supported on that device!" << std::endl;
    }
    if (VkResult result = glfwCreateWindowSurface(instance, window, allocator, &surface); VK_SUCCESS != result) 
    {   
        std::cerr << "glfwCreateWindowSurface() failed with result: " << result << std::endl;
        vkDestroyInstance(instance, nullptr);
    }
}

vk_renderer::~vk_renderer()
{
    glfwDestroyWindow(window);
    vkDestroySurfaceKHR(instance, surface, allocator);
    vkDestroyInstance(instance, nullptr);
}

GLFWwindow *vk_renderer::getWindow() const
{
    return window;
}

void vk_renderer::tick(const float &delta_time)
{
}