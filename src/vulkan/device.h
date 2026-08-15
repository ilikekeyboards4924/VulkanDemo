#pragma once
#include <vulkan/vulkan.h>

class VulkanDevice {
public:
	VulkanDevice(VkInstance instance);
	~VulkanDevice();

	// disable copying
	VulkanDevice(const VulkanDevice&) = delete;
	VulkanDevice& operator=(const VulkanDevice&) = delete;

	VkPhysicalDevice physicalDeviceHandle() const { return m_physicalDevice; }
	VkDevice handle() const { return m_device; }
	VkQueue graphicsQueueHandle() const { return m_graphicsQueue; }
	uint32_t graphicsQueueFamilyIndex() const { return m_graphicsFamilyIndex; }
private:
	VkPhysicalDevice pickPhysicalDevice(VkInstance instance);
	void createLogicalDevice();

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE; // gpu
	VkDevice m_device = VK_NULL_HANDLE; // logical device

	VkQueue m_graphicsQueue = VK_NULL_HANDLE; // graphics and presenting queue?

	uint32_t m_graphicsFamilyIndex;
};