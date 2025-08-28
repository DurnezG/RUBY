#include "Vulkan/Pipeline.h"

#include <array>
#include <stdexcept>

#include "Vulkan/Shader.h"

namespace RUBY
{
    // ---------------- Pipeline Implementation ---------------- //

    Pipeline::Pipeline(Device* device,
        SwapChain* swapChain,
        DescriptorPool* descriptorPool,
        const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages,
        const VkPipelineVertexInputStateCreateInfo& vertexInput,
        const VkPipelineInputAssemblyStateCreateInfo& inputAssembly,
        const VkPipelineViewportStateCreateInfo& viewportState,
        const VkPipelineRasterizationStateCreateInfo& rasterizer,
        const VkPipelineMultisampleStateCreateInfo& multisampling,
        const VkPipelineDepthStencilStateCreateInfo& depthStencil,
        const VkPipelineColorBlendStateCreateInfo& colorBlending,
        const VkPipelineDynamicStateCreateInfo& dynamicState,
        const VkPipelineRenderingCreateInfo& renderingInfo,
        const std::vector<VkPushConstantRange>& pushConstants)
        : m_Device(device), m_SwapChain(swapChain), m_DescriptorPool(descriptorPool)
    {
        // --- Pipeline Layout ---
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        VkDescriptorSetLayout descriptorSetLayout = descriptorPool->GetDescriptorSetLayout(0);
        pipelineLayoutInfo.setLayoutCount = (descriptorSetLayout != VK_NULL_HANDLE) ? 1 : 0;
        pipelineLayoutInfo.pSetLayouts = (descriptorSetLayout != VK_NULL_HANDLE) ? &descriptorSetLayout : nullptr;

        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

        if (vkCreatePipelineLayout(device->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_Layout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        // --- Graphics Pipeline ---
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_Layout;

        // NOTE: Using dynamic rendering, no renderPass needed
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        pipelineInfo.pNext = &renderingInfo;

        if (vkCreateGraphicsPipelines(device->GetLogicalDevice(),
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &m_Pipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }
    }

    Pipeline::Pipeline(Pipeline&& other) noexcept
    {
        *this = std::move(other);
    }

    Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
    {
        if (this != &other)
        {
            m_Device = other.m_Device;
            m_SwapChain = other.m_SwapChain;
            m_DescriptorPool = other.m_DescriptorPool;
            m_Pipeline = other.m_Pipeline;
            m_Layout = other.m_Layout;

            other.m_Pipeline = VK_NULL_HANDLE;
            other.m_Layout = VK_NULL_HANDLE;
        }
        return *this;
    }

    Pipeline::~Pipeline()
    {
        if (m_Pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(m_Device->GetLogicalDevice(), m_Pipeline, nullptr);
        }
        if (m_Layout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(m_Device->GetLogicalDevice(), m_Layout, nullptr);
        }
    }

    // ---------------- PipelineBuilder Implementation ---------------- //

    PipelineBuilder& PipelineBuilder::AddShader(const Shader& shader)
    {
        m_ShaderStages.push_back(shader.GetStageCreateInfo());
        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetVertexInput(const VkPipelineVertexInputStateCreateInfo& vertexInput)
    {
        m_VertexInput = vertexInput;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetInputAssembly(const VkPipelineInputAssemblyStateCreateInfo& inputAssembly)
    {
        m_InputAssembly = inputAssembly;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetViewportState(const VkPipelineViewportStateCreateInfo& viewportState)
    {
        m_ViewportState = viewportState;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetRasterizer(const VkPipelineRasterizationStateCreateInfo& rasterizer)
    {
        m_Rasterizer = rasterizer;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetMultisampling(const VkPipelineMultisampleStateCreateInfo& multisampling)
    {
        m_Multisampling = multisampling;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetDepthStencil(const VkPipelineDepthStencilStateCreateInfo& depthStencil)
    {
        m_DepthStencil = depthStencil;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetColorBlending(const VkPipelineColorBlendStateCreateInfo& colorBlending)
    {
        m_ColorBlending = colorBlending;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetDynamicState(const VkPipelineDynamicStateCreateInfo& dynamicState)
    {
        m_DynamicState = dynamicState;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetRenderingInfo(const VkPipelineRenderingCreateInfo& renderingInfo)
    {
        m_RenderingInfo = renderingInfo;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::AddPushConstant(const VkPushConstantRange& pushConstant)
    {
        m_PushConstants.push_back(pushConstant);
        return *this;
    }

    Pipeline PipelineBuilder::Build(Device* device, SwapChain* swapChain, DescriptorPool* descriptorPool)
    {
        return Pipeline(device,
            swapChain,
            descriptorPool,
            m_ShaderStages,
            m_VertexInput,
            m_InputAssembly,
            m_ViewportState,
            m_Rasterizer,
            m_Multisampling,
            m_DepthStencil,
            m_ColorBlending,
            m_DynamicState,
            m_RenderingInfo,
            m_PushConstants);
    }

    PipelineBuilder PipelineBuilder::CreateDefault(uint32_t width, uint32_t height)
    {
        PipelineBuilder builder{};

        // --- Vertex Input (no vertices for now, e.g. fullscreen triangle/quad) ---
        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        builder.m_VertexInput = vertexInput;

        // --- Input Assembly ---
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        builder.m_InputAssembly = inputAssembly;

        // --- Viewport + Scissor ---
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { width, height };

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
        builder.m_ViewportState = viewportState;

        // --- Rasterizer ---
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // depends on your convention
        rasterizer.depthBiasEnable = VK_FALSE;
        builder.m_Rasterizer = rasterizer;

        // --- Multisampling ---
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        builder.m_Multisampling = multisampling;

        // --- Depth/Stencil (disabled for now) ---
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        builder.m_DepthStencil = depthStencil;

        // --- Color Blending ---
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        builder.m_ColorBlending = colorBlending;

        // --- Dynamic State (viewport/scissor can be overridden later) ---
        static VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;
        builder.m_DynamicState = dynamicState;

        // --- Rendering Info (for dynamic rendering, matches swapchain format) ---
        VkPipelineRenderingCreateInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingInfo.colorAttachmentCount = 1;
        // NOTE: You must set renderingInfo.pColorAttachmentFormats later
        builder.m_RenderingInfo = renderingInfo;

        return builder;
    }
}
