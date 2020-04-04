#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class engine;

class vk_renderer 
{
    public:
    vk_renderer(engine* eng);
    ~vk_renderer();

    void tick(const float& delta_time);

    GLFWwindow* getWindow() const;
    
    protected: 
    
    private:
    engine* _engine;

    GLFWwindow* window;
};
