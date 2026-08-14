#pragma once
#include <vulkan/vulkan.h>

class VulkanDevice {
public:
	VulkanDevice(VkInstance instance);
	~VulkanDevice();

	// disable copying
	VulkanDevice(const VulkanDevice&) = delete;
	VulkanDevice& operator=(const VulkanDevice&) = delete;

	VkPhysicalDevice physicalDeviceHandle() const { return m_physicalDevice; };
	VkDevice handle() const { return m_device; };
	VkQueue graphicsQueueHandle() const { return m_graphicsQueue; };

	VkPhysicalDevice pickPhysicalDevice(VkInstance instance);
	void createLogicalDevice();
private:
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE; // gpu
	VkDevice m_device = VK_NULL_HANDLE; // logical device

	VkQueue m_graphicsQueue = VK_NULL_HANDLE; // graphics and presenting queue?

	uint32_t m_graphicsFamilyIndex;
};