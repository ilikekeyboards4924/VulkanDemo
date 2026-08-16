#include "commandmanager.h"
#include "device.h"
#include <iostream>
#include <stdexcept>

CommandManager::CommandManager(VulkanDevice& device, VulkanSwapchain& swapchain) {
	m_device = device.handle();

	VkCommandPoolCreateInfo commandPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = device.graphicsQueueFamilyIndex(),
	};

	VkResult commandPoolCreationResult = vkCreateCommandPool(device.handle(), &commandPoolCreateInfo, nullptr, &m_commandPool);
	if (commandPoolCreationResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool");
	}

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkResult commandBufferAllocationResult = vkAllocateCommandBuffers(device.handle(), &commandBufferAllocateInfo, &m_commandBuffer);
	if (commandBufferAllocationResult != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffer");
	}

	createSyncObjects(device);
}

CommandManager::~CommandManager() {
	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(m_device, m_presentFinishedSemaphore, nullptr);
	vkDestroyFence(m_device, m_drawFence, nullptr);
}

void CommandManager::drawFrame(VulkanDevice& device, VulkanSwapchain& swapchain, VulkanPipeline& pipeline) {
	// wait for the fence to signal before continuing on to the rest of the drawing logic
	VkResult drawFenceWaitResult = vkWaitForFences(device.handle(), 1, &m_drawFence, VK_TRUE, UINT64_MAX);
	if (drawFenceWaitResult != VK_SUCCESS) {
		throw std::runtime_error("failed to wait for fence");
	}
	vkResetFences(device.handle(), 1, &m_drawFence);

	uint32_t imageIndex;
	VkResult acquireNextImageResult = vkAcquireNextImageKHR(device.handle(), swapchain.handle(), UINT64_MAX, m_presentFinishedSemaphore, nullptr, &imageIndex);
	if (acquireNextImageResult != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire next image in swapchain");
	}

	recordCommandBuffer(pipeline, swapchain, imageIndex);

	VkPipelineStageFlags waitDestinationStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT); // which stage of graphics pipeline to wait in
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_presentFinishedSemaphore,
		.pWaitDstStageMask = &waitDestinationStageMask, // from the docs: "Each entry in the waitStages array corresponds to the semaphore with the same index in pWaitSemaphores"
		.commandBufferCount = 1,
		.pCommandBuffers = &m_commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_renderFinishedSemaphore,
	};

	vkQueueSubmit(device.graphicsQueueHandle(), 1, &submitInfo, m_drawFence);

	VkSwapchainKHR swapchainHandle = swapchain.handle();
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_renderFinishedSemaphore,
		.swapchainCount = 1,
		.pSwapchains = &swapchainHandle,
		.pImageIndices = &imageIndex
	};

	VkResult graphicsQueuePresentResult = vkQueuePresentKHR(device.graphicsQueueHandle(), &presentInfo);
	if (graphicsQueuePresentResult != VK_SUCCESS) {
		throw std::runtime_error("failed to present");
	}
}

void CommandManager::recordCommandBuffer(VulkanPipeline& pipeline, VulkanSwapchain& swapchain, uint32_t imageIndex) {
	VkCommandBufferBeginInfo commandBufferBeginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};
	
	vkBeginCommandBuffer(m_commandBuffer, &commandBufferBeginInfo); // empty CommandBufferBeginInfo because none of it is applicable right now. all the default values are fine

	transitionImageLayout( // transition to a rendering layout
		swapchain,
		imageIndex,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		{},
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
	);

	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }; // what color to use for clearing the image
	VkRenderingAttachmentInfo attachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = swapchain.imageViews()[imageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // what to do before rendering
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE, // what to do after rendering
		.clearValue = clearColor,
	};

	VkRenderingInfo renderingInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = {
			.offset = { 0, 0 },
			.extent = swapchain.extent(),
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachmentInfo,
	};

	vkCmdBeginRendering(m_commandBuffer, &renderingInfo);

	vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle());

	VkViewport viewport{ .width = static_cast<float>(swapchain.extent().width), .height = static_cast<float>(swapchain.extent().height) };
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
	VkRect2D scissor{ .offset = { 0, 0 }, .extent = swapchain.extent() };
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

	vkCmdDraw(m_commandBuffer, 3, 1, 0, 0); // after 300 years of writing boilerplate... A DRAW COMMAND!!!!!!!!

	vkCmdEndRendering(m_commandBuffer);

	transitionImageLayout( // transition to a presenting layout
		swapchain,
		imageIndex,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		{},
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
	);

	vkEndCommandBuffer(m_commandBuffer);
}

void CommandManager::createSyncObjects(VulkanDevice& device) {
	VkSemaphoreCreateInfo semaphoreCreateInfo{ // empty because no relevant flags
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	VkResult renderFinishedSemaphoreCreationResult = vkCreateSemaphore(device.handle(), &semaphoreCreateInfo, nullptr, &m_renderFinishedSemaphore);
	if (renderFinishedSemaphoreCreationResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create the render finished semaphore");
	}
	VkResult presentFinishedSemaphoreCreationResult = vkCreateSemaphore(device.handle(), &semaphoreCreateInfo, nullptr, &m_presentFinishedSemaphore);
	if (presentFinishedSemaphoreCreationResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create the present finished semaphore");
	}
	std::cout << "created render finished and present finished semaphores" << std::endl;

	VkFenceCreateInfo fenceCreateInfo{ // empty because no relevant flags
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT, // make sure the fence is signaled before running the first frame, or the whole program just freezes
	};

	VkResult drawFenceCreation = vkCreateFence(device.handle(), &fenceCreateInfo, nullptr, &m_drawFence);
	if (drawFenceCreation != VK_SUCCESS) {
		throw std::runtime_error("failed to create draw fence");
	}
	std::cout << "created draw fence" << std::endl;
}

void CommandManager::transitionImageLayout( // this function changes image from rendering format to presenting format to etc.
	VulkanSwapchain& swapchain,
	uint32_t imageIndex,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	VkAccessFlags2 srcAccessMask,
	VkAccessFlags2 dstAccessMask,
	VkPipelineStageFlags2 srcStageMask,
	VkPipelineStageFlags2 dstStageMask
) {
	VkImageMemoryBarrier2 barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = srcStageMask,
		.srcAccessMask = srcAccessMask,
		.dstStageMask = dstStageMask,
		.dstAccessMask = dstAccessMask,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = swapchain.images()[imageIndex],
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	VkDependencyInfo dependencyInfo{ // i have no idea what any of this means, all copied straight from vulkan docs
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.dependencyFlags = {}, // why is this empty? not sure. copied from vulkan docs
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier,
	};

	vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
}