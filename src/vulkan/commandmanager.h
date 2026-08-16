#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "device.h"
#include "swapchain.h"
#include "pipeline.h"

class CommandManager {
public:
	CommandManager(VulkanDevice& device, VulkanSwapchain& swapchain);
	~CommandManager();

	// disable copying
	CommandManager(const CommandManager&) = delete;
	CommandManager& operator=(const CommandManager&) = delete;

	void recordCommandBuffer(VulkanPipeline& pipeline, VulkanSwapchain& swapchain, uint32_t imageIndex);
	void drawFrame(VulkanDevice& device, VulkanSwapchain& swapchain, VulkanPipeline& pipeline);
private:
	void createCommandPool(VkDevice device, uint32_t queueFamilyIndex);
	void createCommandBuffers(VkDevice device, VkCommandPool commandPool);

	void createSyncObjects(VkDevice device, std::vector<VkImage> swapchainImages);

	void transitionImageLayout(
		VkImage image,
		uint32_t imageIndex,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkAccessFlags2 srcAccessMask,
		VkAccessFlags2 dstAccessMask,
		VkPipelineStageFlags2 srcStageMask,
		VkPipelineStageFlags2 dstStageMask
	);

	VkDevice m_device;

	VkCommandPool m_commandPool;

	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	std::vector<VkCommandBuffer> m_commandBuffers;

	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkSemaphore> m_presentFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;

	uint32_t m_frameIndex = 0;
};