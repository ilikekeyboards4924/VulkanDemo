#include "pipeline.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <assert.h>

VulkanPipeline::VulkanPipeline(VulkanDevice& device, VulkanSwapchain& swapchain) {
    m_device = device.handle();

    m_vertexShaderModule = createShaderModule(device, readShaderFile("shader/shader.vert.spv"));
    m_fragmentShaderModule = createShaderModule(device, readShaderFile("shader/shader.frag.spv"));

    createGraphicsPipeline(device, swapchain);
}

VulkanPipeline::~VulkanPipeline() {
    vkDestroyShaderModule(m_device, m_vertexShaderModule, nullptr);
    vkDestroyShaderModule(m_device, m_fragmentShaderModule, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
}

void VulkanPipeline::createGraphicsPipeline(VulkanDevice& device, VulkanSwapchain& swapchain) {
    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = m_vertexShaderModule,
        .pName = "main", // entry point of the shader program
    };
    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = m_fragmentShaderModule,
        .pName = "main", // entry point of the shader program
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };


    // START OF FIXED-FUNCTION STATE


    // i dont know what this dynamic state stuff is for
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), // copied from vulkan docs
        .pDynamicStates = dynamicStates.data() // copied from vulkan docs
    };


    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{ // these are all 0/nullptr because the positions are currently hardcoded inside the shader
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };


    VkViewport viewport{ 0.0f, 0.0f, static_cast<float>(swapchain.extent().width), static_cast<float>(swapchain.extent().height), 0.0f, 1.0f };
    VkRect2D scissor{ VkOffset2D{ 0, 0 }, swapchain.extent() };
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{ // reference docs for difference between viewports and scissor rectangles
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizerStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE, // do not clamp the depth. if outside of clipping planes, discard
        .rasterizerDiscardEnable = VK_FALSE, // do not discard the rasterizer result
        .polygonMode = VK_POLYGON_MODE_FILL, // fill with fragments
        .cullMode = VK_CULL_MODE_BACK_BIT, // cull the back faces
        .frontFace = VK_FRONT_FACE_CLOCKWISE, // dont know, copied from vulkan docs
        .depthBiasEnable = VK_FALSE, // add constant value to depth, useful for shadow maps
        .lineWidth = 1.0f, // functionally irrelevant when using polygon_mode_fill, but vulkan gets mad if its not defined
    };

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{ // useful for anti-aliasing. adds complexity, disable it for now
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{ // color blending. set it to false, so that color from fragment shader overwrites current color
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState,
    };

    m_pipelineLayout = nullptr;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0, // dont know, copied from vulkan docs
        .pushConstantRangeCount = 0, // dont know, copied from vulkan docs
    };

    VkResult pipelineLayoutCreationResult = vkCreatePipelineLayout(device.handle(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
    if (pipelineLayoutCreationResult != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }


    // END OF FIXED-FUNCTION STATE

    VkFormat colorAttachmentFormat = swapchain.imageFormat();
    VkPipelineRenderingCreateInfo renderingCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorAttachmentFormat,
    };

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCreateInfo,
        .stageCount = 2, // shader stages
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizerStateCreateInfo,
        .pMultisampleState = &multisampleStateCreateInfo,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = m_pipelineLayout,
        .renderPass = nullptr,
    };

    VkResult graphicsPipelineCreationResult = vkCreateGraphicsPipelines(device.handle(), nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &m_graphicsPipeline);
    if (graphicsPipelineCreationResult != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline");
    }

    std::cout << "created graphics pipeline" << std::endl;
}

std::vector<char> VulkanPipeline::readShaderFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

    file.close();

    std::cout << "read file \"" << filename << "\" of size: " << buffer.size() << std::endl;

    return buffer;
}

VkShaderModule VulkanPipeline::createShaderModule(VulkanDevice& device, const std::vector<char>& code) {
    assert(code.size() % 4 == 0 && "SPIR-V code size must be a multiple of 4");

    VkShaderModuleCreateInfo shaderModuleCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size() * sizeof(char), // copied from vulkan docs
        .pCode = reinterpret_cast<const uint32_t*>(code.data()) // copied from vulkan docs
    };

    VkShaderModule shaderModule;
    VkResult shaderModuleCreationResult = vkCreateShaderModule(device.handle(), &shaderModuleCreateInfo, nullptr, &shaderModule);
    if (shaderModuleCreationResult != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module");
    }

    return shaderModule;
}