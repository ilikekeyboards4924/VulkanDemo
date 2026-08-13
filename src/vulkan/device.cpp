#include "device.h"
#include <vector>
#include <iostream>
#include <stdexcept>

VulkanDevice::VulkanDevice(VkInstance instance) {
	m_physicalDevice = pickPhysicalDevice(instance);
}

VulkanDevice::~VulkanDevice() {
	return;
}

VkPhysicalDevice VulkanDevice::pickPhysicalDevice(VkInstance instance) {
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

	throw std::runtime_error("no discrete gpu found");
}