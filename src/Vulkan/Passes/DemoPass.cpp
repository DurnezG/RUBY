#include "Vulkan/Passes/DemoPass.h"

#include <stdexcept>
#include <array>

namespace RUBY
{
    DemoPass::DemoPass(Device* device, SwapChain* swapchain)
        : m_Device(device), m_SwapChain(swapchain)
    {
        CreateGraphicsPipeline();
    }

    DemoPass::~DemoPass()
    {
        auto dev = m_Device->GetLogicalDevice();
        vkDestroyPipeline(dev, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(dev, m_PipelineLayout, nullptr);
    }

    void DemoPass::Record(VkCommandBuffer cmd, uint32_t imageIndex)
    {
		Image& currentImage = m_SwapChain->GetImages()[imageIndex];

        VkRenderingAttachmentInfoKHR colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        colorAttachment.imageView = currentImage.GetImageView();
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        VkClearValue clearColor = { {0.0f, 0.2f, 0.4f, 1.0f} };
        colorAttachment.clearValue = clearColor;

        VkRenderingInfoKHR renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderingInfo.renderArea.offset = { 0, 0 };
        renderingInfo.renderArea.extent = m_SwapChain->GetExtent();
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

		currentImage.TransitionImageLayout(cmd, currentImage.GetFormat(), currentImage.GetImageLayout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

        vkCmdBeginRendering(cmd, &renderingInfo);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
        vkCmdDraw(cmd, 3, 1, 0, 0);
        vkCmdEndRendering(cmd);

		//currentImage.TransitionImageLayout(cmd, currentImage.GetFormat(), currentImage.GetImageLayout(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_NONE);
    }

    void DemoPass::Recreate(SwapChain* swapchain)
    {
        m_SwapChain = swapchain;
        auto dev = m_Device->GetLogicalDevice();
        vkDestroyPipeline(dev, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(dev, m_PipelineLayout, nullptr);
        CreateGraphicsPipeline();
    }

    void DemoPass::CreateGraphicsPipeline()
    {
	    auto vertCode = Shader::ReadFile("shaders/demo_vert.spv");
	    auto fragCode = Shader::ReadFile("shaders/demo_frag.spv");

	    VkShaderModule vertShaderModule = Shader::CreateShaderModule(m_Device->GetLogicalDevice(), vertCode);
	    VkShaderModule fragShaderModule = Shader::CreateShaderModule(m_Device->GetLogicalDevice(), fragCode);

	    VkPipelineShaderStageCreateInfo shaderStages[2]{};
	    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	    shaderStages[0].module = vertShaderModule;
	    shaderStages[0].pName = "main";

	    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	    shaderStages[1].module = fragShaderModule;
	    shaderStages[1].pName = "main";

	    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	    VkViewport viewport{};
	    viewport.width = (float)m_SwapChain->GetExtent().width;
	    viewport.height = (float)m_SwapChain->GetExtent().height;
	    viewport.minDepth = 0.0f;
	    viewport.maxDepth = 1.0f;

	    VkRect2D scissor{};
	    scissor.extent = m_SwapChain->GetExtent();

	    VkPipelineViewportStateCreateInfo viewportState{};
	    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	    viewportState.viewportCount = 1;
	    viewportState.pViewports = &viewport;
	    viewportState.scissorCount = 1;
	    viewportState.pScissors = &scissor;

	    VkPipelineRasterizationStateCreateInfo rasterizer{};
	    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	    rasterizer.lineWidth = 1.0f;

	    VkPipelineMultisampleStateCreateInfo multisampling{};
	    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	    VkPipelineColorBlendStateCreateInfo colorBlending{};
	    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	    colorBlending.attachmentCount = 1;
	    colorBlending.pAttachments = &colorBlendAttachment;

	    VkPipelineLayoutCreateInfo layoutInfo{};
	    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	    if (vkCreatePipelineLayout(m_Device->GetLogicalDevice(), &layoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
	        throw std::runtime_error("failed to create pipeline layout!");

	    // Dynamic rendering info
	    VkPipelineRenderingCreateInfoKHR renderingInfo{};
	    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	    VkFormat colorFormat = m_SwapChain->GetImageFormat();
	    renderingInfo.colorAttachmentCount = 1;
	    renderingInfo.pColorAttachmentFormats = &colorFormat;

	    VkGraphicsPipelineCreateInfo pipelineInfo{};
	    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	    pipelineInfo.stageCount = 2;
	    pipelineInfo.pStages = shaderStages;
	    pipelineInfo.pVertexInputState = &vertexInputInfo;
	    pipelineInfo.pInputAssemblyState = &inputAssembly;
	    pipelineInfo.pViewportState = &viewportState;
	    pipelineInfo.pRasterizationState = &rasterizer;
	    pipelineInfo.pMultisampleState = &multisampling;
	    pipelineInfo.pColorBlendState = &colorBlending;
	    pipelineInfo.layout = m_PipelineLayout;
	    pipelineInfo.pNext = &renderingInfo;

	    if (vkCreateGraphicsPipelines(m_Device->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
	        throw std::runtime_error("failed to create graphics pipeline!");

	    vkDestroyShaderModule(m_Device->GetLogicalDevice(), vertShaderModule, nullptr);
	    vkDestroyShaderModule(m_Device->GetLogicalDevice(), fragShaderModule, nullptr);
    }

}
