#include "RUBY.h"

#include "Vulkan/Passes/DemoPass.h"

namespace RUBY
{
    RUBY::RUBY(IRubyWindow* pWindow)
        : m_pWindow(pWindow), m_Device(pWindow), m_CommandPool(&m_Device), m_SwapChain(pWindow, &m_Device, &m_CommandPool)
    {
        m_TrianglePass = std::make_unique<DemoPass>(&m_Device, &m_SwapChain);
    }

    RUBY::~RUBY()
    {
        vkDeviceWaitIdle(m_Device.GetLogicalDevice());
        m_TrianglePass.reset();
    }

    void RUBY::Render()
    {
        uint32_t imageIndex;
        if (!BeginFrame(imageIndex)) return;

        VkCommandBuffer cmd = m_CommandPool.GetCommandBuffers()[m_CurrentFrame];
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmd, &beginInfo);

        m_TrianglePass->Record(cmd, imageIndex);

        vkEndCommandBuffer(cmd);
        EndFrame(imageIndex);
    }

    bool RUBY::BeginFrame(uint32_t& outImageIndex)
    {
        vkWaitForFences(m_Device.GetLogicalDevice(), 1, &m_SwapChain.GetInFlightFence(m_CurrentFrame), VK_TRUE, UINT64_MAX);

        VkResult result = vkAcquireNextImageKHR(
            m_Device.GetLogicalDevice(),
            m_SwapChain.GetSwapChain(),
            UINT64_MAX,
            m_SwapChain.GetImageAvailableSemaphore(m_CurrentFrame),
            VK_NULL_HANDLE,
            &outImageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain();
            return false;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(m_Device.GetLogicalDevice(), 1, &m_SwapChain.GetInFlightFence(m_CurrentFrame));
        return true;
    }

    void RUBY::EndFrame(uint32_t imageIndex)
    {
        VkCommandBuffer cmd = m_CommandPool.GetCommandBuffers()[m_CurrentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_SwapChain.GetImageAvailableSemaphore(m_CurrentFrame) };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        VkSemaphore signalSemaphores[] = { m_SwapChain.GetRenderFinishedSemaphore(m_CurrentFrame) };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, m_SwapChain.GetInFlightFence(m_CurrentFrame)) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { m_SwapChain.GetSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        VkResult result = vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized) {
            m_FramebufferResized = false;
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void RUBY::RecreateSwapChain()
    {
        vkDeviceWaitIdle(m_Device.GetLogicalDevice());
        m_SwapChain.RecreateSwapChain();
        m_TrianglePass->Recreate(&m_SwapChain);
    }
}
