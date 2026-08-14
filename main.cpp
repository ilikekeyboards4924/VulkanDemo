#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include "src/vulkan/instance.h"
#include "src/vulkan/device.h"

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "VulkanDemo", nullptr, nullptr);

    VulkanInstance instance;
    
    VkSurfaceKHR surface;
    VkResult surfaceCreationResult = glfwCreateWindowSurface(instance.handle(), window, nullptr, &surface);
    if (surfaceCreationResult != VK_SUCCESS) {
        throw std::runtime_error("failed to create surface");
    }
    
    VulkanDevice device(instance.handle());



    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}