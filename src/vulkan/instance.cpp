#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>
#include "instance.h"

VulkanInstance::VulkanInstance() {
	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "VulkanDemo",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3
	};

	// get required instance extensions from glfw
	uint32_t glfwExtensionCount = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// enable vulkan validation layers for checking errors
	const char* validationLayerName = "VK_LAYER_KHRONOS_validation";

	VkInstanceCreateInfo instanceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,

		// error checking with validation layers (remove for performance)
		.enabledLayerCount = 1,
		.ppEnabledLayerNames = &validationLayerName,

		// enable glfw required extensions
		.enabledExtensionCount = glfwExtensionCount,
		.ppEnabledExtensionNames = glfwExtensions
	};

	VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create vulkan instance");
	}
}

VulkanInstance::~VulkanInstance() {
	vkDestroyInstance(m_instance, nullptr);
}