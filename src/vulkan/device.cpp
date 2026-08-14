#include "device.h"
#include <vector>
#include <iostream>
#include <stdexcept>

VulkanDevice::VulkanDevice(VkInstance instance) {
	m_physicalDevice = pickPhysicalDevice(instance);
	createLogicalDevice();
}

VulkanDevice::~VulkanDevice() {
	vkDestroyDevice(m_device, nullptr);
}

VkPhysicalDevice VulkanDevice::pickPhysicalDevice(VkInstance instance) { // todo: add VkSurfaceKHR surface as a parameter, so that it can check for a "presenting" queue family
	// get a list of the physical devices
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0) throw std::runtime_error("no physical devices found");
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

	// pick the first physicalDevice that is a discrete gpu
	for (auto physicalDevice : physicalDevices) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			return physicalDevice;
		}
	}

	// throw error if none found
	throw std::runtime_error("no discrete gpu found");
}

void VulkanDevice::createLogicalDevice() {
	// get a list of the queue families on the physical device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

	// find the index of the first queue family that supports graphics
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			m_graphicsFamilyIndex = i;
			std::cout << "graphics queue family found at index " << i << std::endl;
			break;
		}
	}

	float queuePriority = 1.0f; // queuePriority is required, even if there's only a single queue
	VkDeviceQueueCreateInfo deviceQueueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = m_graphicsFamilyIndex,
		.queueCount = 1, // only need a single queue for now. maybe more than one for a texture-heavy project
		.pQueuePriorities = &queuePriority
	};

	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo deviceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1, // only one queue family is being set up in the device
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()), // may be unsafe if not casting to uint32_t
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures
	};

	VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device");
	}

	// note about this line: sometimes the "graphics" queue family and the "presenting" queue family are two separate families (could cause issues on things like iGPUs)
	vkGetDeviceQueue(m_device, m_graphicsFamilyIndex, 0, &m_graphicsQueue);

	std::cout << "logical device and graphics queue created" << std::endl;
}