#pragma once
#include <vulkan/vulkan.h>

class VulkanDevice {
public:
	VulkanDevice(VkInstance instance);
	~VulkanDevice();

	VkPhysicalDevice pickPhysicalDevice(VkInstance instance);
private:
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
};