#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class engine;

class vk_renderer
{
public:
    vk_renderer(engine *eng);
    ~vk_renderer();

    void tick(const float &delta_time);

    GLFWwindow *getWindow() const;

protected:
    inline void createWindow();

    inline void createInstance();
    
    inline void createSurface();

    inline void selectPhysicalDevice();
    
    inline void createLogicalDevice();
private:   
    engine *_engine;

    GLFWwindow *window;

    VkAllocationCallbacks* allocator;

    VkInstance instance;

    VkSurfaceKHR surface;

    VkPhysicalDevice mGpu = VK_NULL_HANDLE;

    VkPhysicalDeviceProperties mGpuProperties;
    
    VkPhysicalDeviceFeatures mGpuFeatures;

    VkDevice device = VK_NULL_HANDLE;

    VkQueue mGraphicsQueue;

    
};
