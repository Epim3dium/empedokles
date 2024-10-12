#include "window.hpp"

// std
#include <stdexcept>
#include <utility>

namespace emp {

Window::Window(int w, int h, std::string name)
    : width{w}, height{h}, windowName{std::move(std::move(name))} {
    initWindow();
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(
            width, height, windowName.c_str(), nullptr, nullptr
    );
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
}

void Window::framebufferResizeCallback(
        GLFWwindow* window, int width, int height
) {
    auto pwindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    pwindow->framebufferResized = true;
    pwindow->width = width / 2;
    pwindow->height = height / 2;
}

} // namespace emp
