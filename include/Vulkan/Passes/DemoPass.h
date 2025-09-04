#pragma once
#include <vulkan/vulkan.h>
#include "Vulkan/Device.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/Shader.h"

namespace RUBY
{
    class DemoPass
    {
    public:
        DemoPass(Device* device, SwapChain* swapchain);
        ~DemoPass();

        void Record(VkCommandBuffer cmd, uint32_t imageIndex);
        void Recreate(SwapChain* swapchain);

    private:
        void CreateGraphicsPipeline();

        Device* m_Device;
        SwapChain* m_SwapChain;

        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
    };
}