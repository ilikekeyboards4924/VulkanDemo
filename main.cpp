#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include "src/vulkan/instance.h"
#include "src/vulkan/device.h"
#include "src/vulkan/swapchain.h"
#include "src/vulkan/pipeline.h"


#include <vector>

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
    

    // put these in their own block so that their destructors run before the surface is destroyed
    {
        VulkanDevice device(instance.handle());
        VulkanSwapchain swapchain(device, surface);

        VulkanPipeline pipeline(device, swapchain);
        //pipeline.readShaderFile("src/spirv/vert.spv"); // this will throw an error because the working directory is the build folder, not the main.cpp folder
        std::vector<char> bufferTest = pipeline.readShaderFile("shader/shader.vert.spv");
        std::cout << "read file of size:" << bufferTest.size() << ";" << std::endl;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    vkDestroySurfaceKHR(instance.handle(), surface, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}