#include <vector>
#include <iostream>
#include <stdexcept>
#include "device.h"
#include "swapchain.h"

// m_device is a handle to the logical device, device is the class i created
VulkanSwapchain::VulkanSwapchain(VulkanDevice& device, VkSurfaceKHR surface) {
	m_device = device.handle();
	createSwapchain(device, surface);
	createImageViews(device);
};

VulkanSwapchain::~VulkanSwapchain() {
	for (auto imageView : m_swapchainImageViews) {
		vkDestroyImageView(m_device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

void VulkanSwapchain::pickSurfaceFormat(VulkanDevice& device, VkSurfaceKHR surface) {
	// get a list of surface formats
	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDeviceHandle(), surface, &surfaceFormatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDeviceHandle(), surface, &surfaceFormatCount, surfaceFormats.data());

	// prefer SRGB format (try DCI-P3 at some point)
	for (auto surfaceFormat : surfaceFormats) {
		if (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB) { // windows prefers/expects BGRA and not RGBA for some reason
			m_surfaceFormat = surfaceFormat;
			return; // got an error earlier because i forgot to add this return
		}
	}

	throw std::runtime_error("could not find surface format with color space SRGB and color format BGRA");
}

void VulkanSwapchain::pickExtent(VulkanDevice& device, VkSurfaceKHR surface) {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDeviceHandle(), surface, &surfaceCapabilities);

	m_extent = surfaceCapabilities.currentExtent;
}

void VulkanSwapchain::createSwapchain(VulkanDevice& device, VkSurfaceKHR surface) {
	pickSurfaceFormat(device, surface);
	pickExtent(device, surface);

	// get the glfw surface capabilities
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDeviceHandle(), surface, &surfaceCapabilities);

	VkSwapchainCreateInfoKHR swapchainCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = 2, // should be at least 2 for double buffering/vsync, maybe 3 for triple buffer
		.imageFormat = m_surfaceFormat.format,
		.imageColorSpace = m_surfaceFormat.colorSpace,
		.imageExtent = surfaceCapabilities.currentExtent, // literally just width and height
		.imageArrayLayers = 1, // 1 for majority of standard displays, 2 for VR
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // intend on rendering color into and presenting images
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE, // only one queue family can use the image at a time
		.queueFamilyIndexCount = 0, // ignore since its in exclusive mode
		.pQueueFamilyIndices = nullptr, // ignore since its in exclusive mode
		.preTransform = surfaceCapabilities.currentTransform, // do not rotate the screen at all (use this for mobile devices)
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // window is always fully opaque, cannot see through window to desktop ever
		.presentMode = VK_PRESENT_MODE_FIFO_KHR, // figure this out later (mailbox for triple buffering, but fifo is guaranteed to exist)
		.clipped = VK_TRUE, // vulkan is allowed to discard pixels outside of the viewable area
		.oldSwapchain = VK_NULL_HANDLE // provide old swapchain when resizing window
	};

	VkResult swapchainCreationResult = vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain);
	if (swapchainCreationResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create swapchain");
	}
	std::cout << "created swapchain with extent width:" << surfaceCapabilities.currentExtent.width << ";height:" << surfaceCapabilities.currentExtent.height << std::endl;

	uint32_t swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(device.handle(), m_swapchain, &swapchainImageCount, nullptr);
	m_swapchainImages.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(device.handle(), m_swapchain, &swapchainImageCount, m_swapchainImages.data());
}

void VulkanSwapchain::createImageViews(VulkanDevice& device) {
	m_swapchainImageViews.resize(m_swapchainImages.size());

	for (uint32_t i = 0; i < m_swapchainImages.size(); i++) {
		VkImageViewCreateInfo imageViewCreateInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = m_swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = m_surfaceFormat.format,
			.subresourceRange = { // dont really know what any of this means
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};

		vkCreateImageView(device.handle(), &imageViewCreateInfo, nullptr, &m_swapchainImageViews[i]);
	}
}