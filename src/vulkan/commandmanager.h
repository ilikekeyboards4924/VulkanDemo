#pragma once
#include <vulkan/vulkan.h>
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
	void createSyncObjects(VulkanDevice& device);

	void transitionImageLayout(
		VulkanSwapchain& swapchain,
		uint32_t imageIndex,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkAccessFlags2 srcAccessMask,
		VkAccessFlags2 dstAccessMask,
		VkPipelineStageFlags2 srcStageMask,
		VkPipelineStageFlags2 dstStageMask
	);

	VkDevice m_device; // use for destruction

	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;

	// (i think?) semaphores do not block CPU execution, fences block CPU execution
	VkSemaphore m_renderFinishedSemaphore;
	VkSemaphore m_presentFinishedSemaphore;
	VkFence m_drawFence;
};