#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "device.h"
#include "swapchain.h"

class VulkanPipeline {
public:
	VulkanPipeline(VulkanDevice& device, VulkanSwapchain& swapchain);
	~VulkanPipeline();

	std::vector<char> readShaderFile(const std::string& filename);
private:
	VkShaderModule createShaderModule(VulkanDevice& device, const std::vector<char>& code);

	VkDevice m_device; // use for destruction
	VkShaderModule m_vertexShaderModule;
	VkShaderModule m_fragmentShaderModule;

	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_graphicsPipeline;
};