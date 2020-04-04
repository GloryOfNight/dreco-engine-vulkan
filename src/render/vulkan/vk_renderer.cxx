#include "vk_renderer.hxx"

vk_renderer::vk_renderer(engine* eng) : _engine(eng)
{
    window = glfwCreateWindow(540, 720, "engine", nullptr, nullptr);
}

vk_renderer::~vk_renderer()
{
    glfwDestroyWindow(window);
}

GLFWwindow* vk_renderer::getWindow() const 
{
    return window;
}

void vk_renderer::tick(const float& delta_time) 
{

}