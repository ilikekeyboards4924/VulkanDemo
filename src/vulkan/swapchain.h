#pragma once
#include <vulkan/vulkan.h>

class VulkanSwapchain {
public:
	VulkanSwapchain();
	~VulkanSwapchain();

	// disable copying
	VulkanSwapchain(const VulkanSwapchain&) = delete;
	VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

private:

};