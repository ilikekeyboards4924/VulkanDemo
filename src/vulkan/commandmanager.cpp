#include "commandmanager.h"
#include "device.h"
#include <iostream>
#include <stdexcept>
#include <assert.h>

CommandManager::CommandManager(VulkanDevice& device, VulkanSwapchain& swapchain) {
	m_device = device.handle();

	createCommandPool(device.handle(), device.graphicsQueueFamilyIndex());
	createCommandBuffers(device.handle(), m_commandPool);
	createSyncObjects(device.handle(), swapchain.images());
}

void CommandManager::createCommandPool(VkDevice device, uint32_t queueFamilyIndex) {
	VkCommandPoolCreateInfo commandPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queueFamilyIndex,
	};

	VkResult result = vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &m_commandPool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool");
	}
}

void CommandManager::createCommandBuffers(VkDevice device, VkCommandPool commandPool) {
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT, // one command buffer per frame
	};

	m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	VkResult result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, m_commandBuffers.data());
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers");
	}
}

void CommandManager::createSyncObjects(VkDevice device, std::vector<VkImage> swapchainImages) {
	VkSemaphoreCreateInfo semaphoreCreateInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VkFenceCreateInfo fenceCreateInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT }; // if it doesnt start signaled, the program will just freeze

	assert(m_renderFinishedSemaphores.empty() && m_inFlightFences.empty() && m_presentFinishedSemaphores.empty());

	// renderFinishedSemaphores will be using the imageIndex, they are tied to the specific images
	// presentFinishedSemaphores will be using the m_frameIndex

	for (uint32_t i = 0; i < swapchainImages.size(); i++) {
		VkSemaphore semaphore;
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);
		m_renderFinishedSemaphores.push_back(semaphore);
	}
	std::cout << "created render finished semaphores" << std::endl;
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkSemaphore semaphore;
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);
		m_presentFinishedSemaphores.push_back(semaphore);

		VkFence fence;
		vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
		m_inFlightFences.push_back(fence);
	}
	std::cout << "created present finished semaphores and in-flight fences" << std::endl;
}


void CommandManager::drawFrame(VulkanDevice& device, VulkanSwapchain& swapchain, VulkanPipeline& pipeline) {
	VkResult fenceResult = vkWaitForFences(device.handle(), 1, &m_inFlightFences[m_frameIndex], VK_TRUE, UINT64_MAX);
	if (fenceResult != VK_SUCCESS) {
		throw std::runtime_error("failed to wait for fence");
	}
	vkResetFences(device.handle(), 1, &m_inFlightFences[m_frameIndex]);

	uint32_t imageIndex;
	VkResult nextImageResult = vkAcquireNextImageKHR(device.handle(), swapchain.handle(), UINT64_MAX, m_presentFinishedSemaphores[m_frameIndex], nullptr, &imageIndex);
	if (nextImageResult != VK_SUCCESS) {
		std::cout << "vkAcquireNextImageKHR returned: " << nextImageResult << std::endl;
		throw std::runtime_error("failed to acquire next image in swapchain");
	}

	vkResetCommandBuffer(m_commandBuffers[m_frameIndex], 0);
	recordCommandBuffer(pipeline, swapchain, imageIndex);

	VkPipelineStageFlags waitDestinationStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT); // allow pipeline to work on vertex shaders and halt on the fragment shader stage
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_presentFinishedSemaphores[m_frameIndex],
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &m_commandBuffers[m_frameIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_renderFinishedSemaphores[imageIndex]
	};

	vkQueueSubmit(device.graphicsQueueHandle(), 1, &submitInfo, m_inFlightFences[m_frameIndex]);

	m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

	VkSwapchainKHR swapchainHandle = swapchain.handle();
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_renderFinishedSemaphores[imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &swapchainHandle,
		.pImageIndices = &imageIndex,
	};

	VkResult presentResult = vkQueuePresentKHR(device.graphicsQueueHandle(), &presentInfo);
	if (presentResult != VK_SUCCESS) {
		throw std::runtime_error("failed to present");
	}
}

void CommandManager::recordCommandBuffer(VulkanPipeline& pipeline, VulkanSwapchain& swapchain, uint32_t imageIndex) {
	VkCommandBufferBeginInfo commandBufferBeginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, };
	
	vkBeginCommandBuffer(m_commandBuffers[m_frameIndex], &commandBufferBeginInfo); // empty CommandBufferBeginInfo because none of it is applicable right now. all the default values are fine

	transitionImageLayout( // transition to a rendering layout
		swapchain.images()[imageIndex],
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

	vkCmdBeginRendering(m_commandBuffers[m_frameIndex], &renderingInfo);

	vkCmdBindPipeline(m_commandBuffers[m_frameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle());

	VkViewport viewport{ .width = static_cast<float>(swapchain.extent().width), .height = static_cast<float>(swapchain.extent().height) };
	vkCmdSetViewport(m_commandBuffers[m_frameIndex], 0, 1, &viewport);
	VkRect2D scissor{ .offset = { 0, 0 }, .extent = swapchain.extent() };
	vkCmdSetScissor(m_commandBuffers[m_frameIndex], 0, 1, &scissor);

	vkCmdDraw(m_commandBuffers[m_frameIndex], 6, 1, 0, 0); // after 300 years of writing boilerplate... A DRAW COMMAND!!!!!!!!

	vkCmdEndRendering(m_commandBuffers[m_frameIndex]);

	transitionImageLayout( // transition to a presenting layout
		swapchain.images()[imageIndex],
		imageIndex,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		{},
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
	);

	vkEndCommandBuffer(m_commandBuffers[m_frameIndex]);
}


void CommandManager::transitionImageLayout( // this function changes image from rendering format to presenting format to etc.
	VkImage image,
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
		.image = image,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	// container/wrapper that packages multiple synchronization rules into one dependency struct
	VkDependencyInfo dependencyInfo{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.dependencyFlags = {}, // used for render passes (and more specifically tile-based mobile gpus)
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier,
	};
	
	// called a barrier because it blocks gpu from doing anything else, until its properly finished with the old format.
	// used for synchronization
	vkCmdPipelineBarrier2(m_commandBuffers[m_frameIndex], &dependencyInfo);
}

CommandManager::~CommandManager() {
	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	for (uint32_t i = 0; i < m_renderFinishedSemaphores.size(); i++) {
		vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
	}
	for (uint32_t i = 0; i < m_presentFinishedSemaphores.size(); i++) {
		vkDestroySemaphore(m_device, m_presentFinishedSemaphores[i], nullptr);
	}
	for (uint32_t i = 0; i < m_inFlightFences.size(); i++) {
		vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
	}
}