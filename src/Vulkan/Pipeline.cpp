#include "Vulkan/Pipeline.h"

#include <stdexcept>
#include <utility>

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
        VkPipelineRenderingCreateInfo renderingInfo,
        const std::vector<VkPushConstantRange>& pushConstants)
        : m_Device(device), m_SwapChain(swapChain), m_DescriptorPool(descriptorPool)
    {
        // Create pipeline layout (use ALL descriptor set layouts from the DescriptorPool)
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        const auto& setLayouts = descriptorPool->GetDescriptorSetLayouts();
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        pipelineLayoutInfo.pSetLayouts = setLayouts.empty() ? nullptr : setLayouts.data();

        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.empty() ? nullptr : pushConstants.data();

        if (vkCreatePipelineLayout(device->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_Layout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        // Build graphics pipeline create info (using dynamic rendering via pNext)
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.empty() ? nullptr : shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_Layout;

        // No renderPass when using dynamic rendering
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        // Hook up rendering info via pNext
        pipelineInfo.pNext = &renderingInfo;

        VkPipelineCache pipelineCache = VK_NULL_HANDLE; // optional: supply a cache if you have one
        if (vkCreateGraphicsPipelines(device->GetLogicalDevice(), pipelineCache, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
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
            // Destroy existing
            if (m_Layout != VK_NULL_HANDLE && m_Device)
            {
                vkDestroyPipelineLayout(m_Device->GetLogicalDevice(), m_Layout, nullptr);
                m_Layout = VK_NULL_HANDLE;
            }
            if (m_Pipeline != VK_NULL_HANDLE && m_Device)
            {
                vkDestroyPipeline(m_Device->GetLogicalDevice(), m_Pipeline, nullptr);
                m_Pipeline = VK_NULL_HANDLE;
            }

            m_Device = other.m_Device;
            m_SwapChain = other.m_SwapChain;
            m_DescriptorPool = other.m_DescriptorPool;
            m_Pipeline = std::exchange(other.m_Pipeline, VK_NULL_HANDLE);
            m_Layout = std::exchange(other.m_Layout, VK_NULL_HANDLE);
        }
        return *this;
    }

    Pipeline::~Pipeline()
    {
        if (m_Layout != VK_NULL_HANDLE && m_Device)
        {
            vkDestroyPipelineLayout(m_Device->GetLogicalDevice(), m_Layout, nullptr);
            m_Layout = VK_NULL_HANDLE;
        }
        if (m_Pipeline != VK_NULL_HANDLE && m_Device)
        {
            vkDestroyPipeline(m_Device->GetLogicalDevice(), m_Pipeline, nullptr);
            m_Pipeline = VK_NULL_HANDLE;
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
        // Ensure rendering info has correct attachment formats from swapchain
        m_ColorAttachmentFormats.clear();
        m_ColorAttachmentFormats.push_back(swapChain->GetImageFormat());
        m_RenderingInfo.colorAttachmentCount = static_cast<uint32_t>(m_ColorAttachmentFormats.size());
        m_RenderingInfo.pColorAttachmentFormats = m_ColorAttachmentFormats.data();

        // If depth is enabled in depthStencil, set depthAttachmentFormat accordingly (optional)
        // VkFormat depthFmt = device->FindDepthFormat();
        // m_RenderingInfo.depthAttachmentFormat = depthFmt;

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

        // Vertex input (empty)
        builder.m_VertexInput = VkPipelineVertexInputStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

        // Input assembly
        builder.m_InputAssembly = VkPipelineInputAssemblyStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        builder.m_InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        builder.m_InputAssembly.primitiveRestartEnable = VK_FALSE;

        // Owned viewport/scissor
        builder.m_Viewports = { VkViewport{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f} };
        builder.m_Scissors = { VkRect2D{ {0,0}, { width, height } } };

        builder.m_ViewportState = VkPipelineViewportStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        builder.m_ViewportState.viewportCount = static_cast<uint32_t>(builder.m_Viewports.size());
        builder.m_ViewportState.pViewports = builder.m_Viewports.data();
        builder.m_ViewportState.scissorCount = static_cast<uint32_t>(builder.m_Scissors.size());
        builder.m_ViewportState.pScissors = builder.m_Scissors.data();

        // Rasterizer
        builder.m_Rasterizer = VkPipelineRasterizationStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        builder.m_Rasterizer.depthClampEnable = VK_FALSE;
        builder.m_Rasterizer.rasterizerDiscardEnable = VK_FALSE;
        builder.m_Rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        builder.m_Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        builder.m_Rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        builder.m_Rasterizer.lineWidth = 1.0f;

        // Multisampling
        builder.m_Multisampling = VkPipelineMultisampleStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        builder.m_Multisampling.sampleShadingEnable = VK_FALSE;
        builder.m_Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Depth/stencil default disabled
        builder.m_DepthStencil = VkPipelineDepthStencilStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        builder.m_DepthStencil.depthTestEnable = VK_FALSE;
        builder.m_DepthStencil.depthWriteEnable = VK_FALSE;
        builder.m_DepthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;

        // Color blending (owned attachment)
        builder.m_ColorBlendAttachments = { VkPipelineColorBlendAttachmentState{} };
        builder.m_ColorBlendAttachments[0].blendEnable = VK_FALSE;
        builder.m_ColorBlendAttachments[0].colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        builder.m_ColorBlending = VkPipelineColorBlendStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        builder.m_ColorBlending.logicOpEnable = VK_FALSE;
        builder.m_ColorBlending.attachmentCount = static_cast<uint32_t>(builder.m_ColorBlendAttachments.size());
        builder.m_ColorBlending.pAttachments = builder.m_ColorBlendAttachments.data();

        // Dynamic state (owned)
        builder.m_DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        builder.m_DynamicState = VkPipelineDynamicStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        builder.m_DynamicState.dynamicStateCount = static_cast<uint32_t>(builder.m_DynamicStates.size());
        builder.m_DynamicState.pDynamicStates = builder.m_DynamicStates.data();

        // Rendering info: color attachments set by Build()
        builder.m_RenderingInfo = VkPipelineRenderingCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        builder.m_RenderingInfo.colorAttachmentCount = 0;
        builder.m_RenderingInfo.pColorAttachmentFormats = nullptr;

        return builder;
    }
}
