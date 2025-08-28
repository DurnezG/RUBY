#include "RUBY.h"

RUBY::RUBY::RUBY(IRubyWindow* pWindow)
	: m_pWindow(pWindow)
{
	CreateSyncObjects();
}

RUBY::RUBY::~RUBY()
{
	vkDeviceWaitIdle(m_Device.GetLogicalDevice());
	vkDestroySampler(m_Device.GetLogicalDevice(), m_TextureSampler, nullptr);

    for (size_t i = 0; i < m_RenderFinishedSemaphores.size(); i++)
    {
        vkDestroySemaphore(m_Device.GetLogicalDevice(), m_RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_Device.GetLogicalDevice(), m_ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_Device.GetLogicalDevice(), m_InFlightFences[i], nullptr);
    }
}

void RUBY::RUBY::Render()
{
    uint32_t imageIndex;
    if (!BeginFrame(imageIndex)) return;

    UpdateBuffers();

    VkCommandBuffer cmd = m_CommandPool.GetCommandBuffers()[m_CurrentFrame];
    RecordCommandBuffer(cmd, imageIndex);

    EndFrame(imageIndex);
}

bool RUBY::RUBY::BeginFrame(uint32_t& outImageIndex)
{
    vkWaitForFences(m_Device.GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(
        m_Device.GetLogicalDevice(),
        m_SwapChain.GetSwapChain(),
        UINT64_MAX,
        m_ImageAvailableSemaphores[m_CurrentFrame],
        VK_NULL_HANDLE,
        &outImageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        RecreateSwapChain();
        return false;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(m_Device.GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

    // reset command buffer for this frame
    vkResetCommandBuffer(m_CommandPool.GetCommandBuffers()[m_CurrentFrame], 0);

    return true;
}

VkSampler RUBY::RUBY::CreateTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_Device.GetPhysicalDevice(), &properties);

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(m_Device.GetLogicalDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
    return m_TextureSampler;
}

void RUBY::RUBY::CreateSyncObjects()
{
    m_ImageAvailableSemaphores.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(m_Device.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_Device.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_Device.GetLogicalDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
        {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void RUBY::RUBY::RecreateSwapChain()
{
    m_SwapChain.RecreateSwapChain();
}

void RUBY::RUBY::UpdateBuffers()
{
    for (auto& pass : m_Passes)
    {
        pass.Update(m_CurrentFrame);
    }
}

void RUBY::RUBY::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

	PassContext passContext{};

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    for (auto& pass : m_Passes)
    {
		pass.RecordCommandBuffer(commandBuffer, imageIndex, passContext);
    }

    //   m_DepthPrePass.RecordCommandBuffer(commandBuffer, imageIndex, &m_Scene);
    //   m_BaseRenderPass.RecordCommandBuffer(commandBuffer, imageIndex, &m_Scene, m_DepthPrePass.GetDepthImage(), &m_SwapChain.GetImages()[imageIndex]);
       //m_LightPass.RecordCommandBuffer(commandBuffer, imageIndex, &m_Scene, m_DepthPrePass.GetDepthImage(), &m_SwapChain.GetImages()[imageIndex]);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

void RUBY::RUBY::EndFrame(uint32_t imageIndex)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandPool.GetCommandBuffers()[m_CurrentFrame];

    VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) 
    {
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

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized) 
    {
        m_FramebufferResized = false;
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    frameCount++;
}
