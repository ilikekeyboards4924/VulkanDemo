#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VulkanSwapchain {
public:
	VulkanSwapchain(VulkanDevice& device, VkSurfaceKHR surface);
	~VulkanSwapchain();

	// disable copying
	VulkanSwapchain(const VulkanSwapchain&) = delete;
	VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

	VkSwapchainKHR handle() const { return m_swapchain; }

	const std::vector<VkImageView>& imageViews() const { return m_swapchainImageViews; }
	const std::vector<VkImage>& images() const { return m_swapchainImages; }

	VkFormat imageFormat() const { return m_surfaceFormat.format; }
	VkExtent2D extent() const { return m_extent; }

private:
	void pickSurfaceFormat(VulkanDevice& device, VkSurfaceKHR surface);
	void pickExtent(VulkanDevice& device, VkSurfaceKHR surface); // extent is the dimensions/size

	void createSwapchain(VulkanDevice& device, VkSurfaceKHR surface);
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> m_swapchainImages;

	void createImageViews(VulkanDevice& device);
	std::vector<VkImageView> m_swapchainImageViews;

	VkDevice m_device = VK_NULL_HANDLE; // keep a "copy" of device for proper destruction? seems like a bad idea (i think its okay since it's the handle of the device, not the actual device)
	
	VkSurfaceFormatKHR m_surfaceFormat{}; // empty initialize
	VkExtent2D m_extent{}; // empty initialize
};