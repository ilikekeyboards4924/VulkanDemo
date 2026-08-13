#pragma once
#include <vulkan/vulkan.h>

class VulkanInstance {
public:
	// raii (acquisition and destruction)
	VulkanInstance();
	~VulkanInstance();

	// disable copying
	VulkanInstance(const VulkanInstance&) = delete;
	VulkanInstance& operator=(const VulkanInstance&) = delete;

	VkInstance handle() const { return m_instance; }
private:
	VkInstance m_instance = VK_NULL_HANDLE;
};