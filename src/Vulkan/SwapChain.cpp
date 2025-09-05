#include "Vulkan/SwapChain.h"

#include <algorithm>
#include <stdexcept>
#include <limits>

#include "Vulkan/Device.h"
#include "Vulkan/CommandPool.h"

namespace RUBY
{
    SwapChain::SwapChain(IRubyWindow* window, Device* device, CommandPool* pCommandPool, PresentMode preferredPresentMode)
        : m_pWindow(window), m_pDevice(device), m_pCommandPool(pCommandPool), m_PresentMode(preferredPresentMode)
    {
        CreateSwapChain();
    }

    SwapChain::~SwapChain()
    {
        vkDeviceWaitIdle(m_pDevice->GetLogicalDevice());
        CleanupSwapChainInternal();
    }

    void SwapChain::QuerySwapChainSupport()
    {
        // Query fresh
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_pDevice->GetPhysicalDevice(), m_pDevice->GetSurface(), &m_SwapChainSupport.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_pDevice->GetPhysicalDevice(), m_pDevice->GetSurface(), &formatCount, nullptr);
        m_SwapChainSupport.formats.clear();
        if (formatCount != 0) {
            m_SwapChainSupport.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_pDevice->GetPhysicalDevice(), m_pDevice->GetSurface(), &formatCount, m_SwapChainSupport.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice(), m_pDevice->GetSurface(), &presentModeCount, nullptr);
        m_SwapChainSupport.presentModes.clear();
        if (presentModeCount != 0) {
            m_SwapChainSupport.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice(), m_pDevice->GetSurface(), &presentModeCount, m_SwapChainSupport.presentModes.data());
        }
    }

    VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
    {
        // TODO: Make swap avaialble between the 2 images
        return VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }
        // fallback
        return availableFormats.empty() ? VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } : availableFormats[0];
    }

    VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const
    {
        // Preference order based on m_PresentMode
        if (m_PresentMode == PresentMode::MAILBOX)
        {
            for (auto mode : availablePresentModes)
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
            // fallthrough to IMMEDIATE, then FIFO
        }
        if (m_PresentMode == PresentMode::IMMEDIATE)
        {
            for (auto mode : availablePresentModes)
                if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) return mode;
        }
        if (m_PresentMode == PresentMode::RELAXED)
        {
            for (auto mode : availablePresentModes)
                if (mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) return mode;
        }

        // Default guaranteed mode
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            m_pWindow->GetFramebufferSize(&width, &height);
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    void SwapChain::CreateSwapChain()
    {
        // Query support
        QuerySwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(m_SwapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(m_SwapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(m_SwapChainSupport.capabilities);

        // Recommend at least min + 1 images
        uint32_t imageCount = m_SwapChainSupport.capabilities.minImageCount + 1;
        if (m_SwapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > m_SwapChainSupport.capabilities.maxImageCount)
        {
            imageCount = m_SwapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_pDevice->GetSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // allow blits if needed

        auto indices = m_pDevice->FindQueueFamilies(m_pDevice->GetPhysicalDevice());
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = m_SwapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = m_SwapChain; // allow efficient resource reuse

        VkSwapchainKHR oldSwapChain = m_SwapChain;
        VkResult result = vkCreateSwapchainKHR(m_pDevice->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChain);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        // If we had an old swapchain, destroy it after creation to allow driver to recycle
        if (oldSwapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(m_pDevice->GetLogicalDevice(), oldSwapChain, nullptr);
        }

        // Get images
        uint32_t actualImageCount = 0;
        vkGetSwapchainImagesKHR(m_pDevice->GetLogicalDevice(), m_SwapChain, &actualImageCount, nullptr);
        std::vector<VkImage> swapChainImages(actualImageCount);
        vkGetSwapchainImagesKHR(m_pDevice->GetLogicalDevice(), m_SwapChain, &actualImageCount, swapChainImages.data());

        m_SwapChainImages.clear();
        m_SwapChainImages.reserve(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            // create Image wrapper around existing VkImage (no allocation)
            m_SwapChainImages.emplace_back(Image{ m_pDevice, m_pCommandPool, swapChainImages[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT });
            m_pDevice->GetDebugger().SetDebugName(reinterpret_cast<uint64_t>(m_SwapChainImages.back().GetImage()), "SwapChain Image", VK_OBJECT_TYPE_IMAGE);
        }

        m_SwapChainImageFormat = surfaceFormat.format;
        m_SwapChainExtent = extent;

        // recreate sync objects (destroy old then create new)
        CleanupSyncObjects();
        CreateSyncObjects();
    }

    void SwapChain::CreateSyncObjects()
    {
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // start signaled so first frame doesn't wait

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (vkCreateSemaphore(m_pDevice->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_pDevice->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_pDevice->GetLogicalDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create synchronization objects for a frame!");
            }
        }
    }

    void SwapChain::CleanupSyncObjects()
    {
        for (size_t i = 0; i < m_ImageAvailableSemaphores.size(); ++i)
        {
            if (m_ImageAvailableSemaphores[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(m_pDevice->GetLogicalDevice(), m_ImageAvailableSemaphores[i], nullptr);
            if (m_RenderFinishedSemaphores[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(m_pDevice->GetLogicalDevice(), m_RenderFinishedSemaphores[i], nullptr);
            if (m_InFlightFences[i] != VK_NULL_HANDLE)
                vkDestroyFence(m_pDevice->GetLogicalDevice(), m_InFlightFences[i], nullptr);
        }
        m_ImageAvailableSemaphores.clear();
        m_RenderFinishedSemaphores.clear();
        m_InFlightFences.clear();
    }

    void SwapChain::CleanupSwapChainInternal()
    {
        // Destroy images (their destructors will free VMA/Views if owned)
        m_SwapChainImages.clear();

        // Sync objects
        CleanupSyncObjects();

        if (m_SwapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(m_pDevice->GetLogicalDevice(), m_SwapChain, nullptr);
            m_SwapChain = VK_NULL_HANDLE;
        }
    }

    uint32_t SwapChain::AcquireNextImage(uint64_t timeout, uint32_t /*frameIndex*/, VkSemaphore signalSemaphore, VkFence fence, VkResult* outResult) const
    {
        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(
            m_pDevice->GetLogicalDevice(),
            m_SwapChain,
            timeout,
            signalSemaphore,
            fence,
            &imageIndex
        );

        if (outResult) *outResult = result;
        return imageIndex;
    }

    void SwapChain::RecreateSwapChain()
    {
        int width = m_pWindow->GetWidth();
        int height = m_pWindow->GetHeight();

        // Wait until non-zero size (minimized windows)
        while (width == 0 || height == 0)
        {
            width = m_pWindow->GetWidth();
            height = m_pWindow->GetHeight();
            m_pWindow->WaitForEvents();
        }

        vkDeviceWaitIdle(m_pDevice->GetLogicalDevice());

        CleanupSwapChainInternal();

        CreateSwapChain();
    }
}
