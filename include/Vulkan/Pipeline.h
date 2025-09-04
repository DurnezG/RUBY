#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include "Vulkan/DescriptorPool.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/Device.h"
#include "Vulkan/Shader.h"

namespace RUBY
{
    class Pipeline
    {
    public:
        Pipeline() = default;

        // Note: renderingInfo is passed by value to ensure its lifetime for pipeline creation.
        Pipeline(Device* device,
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
            const std::vector<VkPushConstantRange>& pushConstants);

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        Pipeline(Pipeline&& other) noexcept;
        Pipeline& operator=(Pipeline&& other) noexcept;

        ~Pipeline();

        VkPipeline GetVkPipeline() const { return m_Pipeline; }
        VkPipelineLayout GetLayout() const { return m_Layout; }

    private:
        Device* m_Device{ nullptr };
        SwapChain* m_SwapChain{ nullptr };
        DescriptorPool* m_DescriptorPool{ nullptr };

        VkPipeline m_Pipeline{ VK_NULL_HANDLE };
        VkPipelineLayout m_Layout{ VK_NULL_HANDLE };
    };

    class PipelineBuilder
    {
    public:
        PipelineBuilder& AddShader(const Shader& shader);
        PipelineBuilder& SetVertexInput(const VkPipelineVertexInputStateCreateInfo& vertexInput);
        PipelineBuilder& SetInputAssembly(const VkPipelineInputAssemblyStateCreateInfo& inputAssembly);
        PipelineBuilder& SetViewportState(const VkPipelineViewportStateCreateInfo& viewportState);
        PipelineBuilder& SetRasterizer(const VkPipelineRasterizationStateCreateInfo& rasterizer);
        PipelineBuilder& SetMultisampling(const VkPipelineMultisampleStateCreateInfo& multisampling);
        PipelineBuilder& SetDepthStencil(const VkPipelineDepthStencilStateCreateInfo& depthStencil);
        PipelineBuilder& SetColorBlending(const VkPipelineColorBlendStateCreateInfo& colorBlending);
        PipelineBuilder& SetDynamicState(const VkPipelineDynamicStateCreateInfo& dynamicState);
        PipelineBuilder& SetRenderingInfo(const VkPipelineRenderingCreateInfo& renderingInfo);
        PipelineBuilder& AddPushConstant(const VkPushConstantRange& pushConstant);

        Pipeline Build(Device* device, SwapChain* swapChain, DescriptorPool* descriptorPool);

        static PipelineBuilder CreateDefault(uint32_t width, uint32_t height);

    private:
        // CreateInfo copies (the create-info structs will point to owned vectors below)
        std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages{};
        VkPipelineVertexInputStateCreateInfo m_VertexInput{};
        VkPipelineInputAssemblyStateCreateInfo m_InputAssembly{};
        VkPipelineViewportStateCreateInfo m_ViewportState{};
        VkPipelineRasterizationStateCreateInfo m_Rasterizer{};
        VkPipelineMultisampleStateCreateInfo m_Multisampling{};
        VkPipelineDepthStencilStateCreateInfo m_DepthStencil{};
        VkPipelineColorBlendStateCreateInfo m_ColorBlending{};
        VkPipelineDynamicStateCreateInfo m_DynamicState{};
        VkPipelineRenderingCreateInfo m_RenderingInfo{};
        std::vector<VkPushConstantRange> m_PushConstants{};

        // Owned storage for arrays pointed to by create-info structs
        std::vector<VkViewport> m_Viewports;
        std::vector<VkRect2D> m_Scissors;
        std::vector<VkPipelineColorBlendAttachmentState> m_ColorBlendAttachments;
        std::vector<VkDynamicState> m_DynamicStates;
        std::vector<VkFormat> m_ColorAttachmentFormats;
    };
}
