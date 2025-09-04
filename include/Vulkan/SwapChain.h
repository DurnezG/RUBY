#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include "Vulkan/Image.h"
#include "Vulkan/Device.h"
#include "Vulkan/CommandPool.h"
#include "Vulkan/IRubyWindow.h"

namespace RUBY
{
    class SwapChain
    {
    public:
        enum class PresentMode
        {
            MAILBOX,
            IMMEDIATE,
            FIFO,
            RELAXED
        };

        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        SwapChain(IRubyWindow* window, Device* device, CommandPool* pCommandPool, PresentMode preferredPresentMode = PresentMode::FIFO);
        ~SwapChain();

        VkSwapchainKHR GetSwapChain() const { return m_SwapChain; }
        VkExtent2D GetExtent() const { return m_SwapChainExtent; }
        VkFormat GetImageFormat() const { return m_SwapChainImageFormat; }
        const std::vector<Image>& GetImages() const { return m_SwapChainImages; }

        // Sync accessors (per-frame)
        VkSemaphore& GetImageAvailableSemaphore(uint32_t frameIndex) { return m_ImageAvailableSemaphores.at(frameIndex); }
        VkSemaphore& GetRenderFinishedSemaphore(uint32_t frameIndex) { return m_RenderFinishedSemaphores.at(frameIndex); }
        VkFence&     GetInFlightFence(uint32_t frameIndex) { return m_InFlightFences.at(frameIndex); }

        uint32_t AcquireNextImage(uint64_t timeout, uint32_t frameIndex, VkSemaphore signalSemaphore, VkFence fence, VkResult* outResult = nullptr) const;

        void RecreateSwapChain();

        PresentMode GetPresentMode() const { return m_PresentMode; }
        void SetPresentMode(PresentMode mode) { m_PresentMode = mode; RecreateSwapChain(); }

    private:
        void CreateSwapChain();
        void QuerySwapChainSupport();

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

        void CreateSyncObjects();
        void CleanupSyncObjects();
        void CleanupSwapChainInternal();

    private:
        IRubyWindow* m_pWindow{ nullptr };
        Device* m_pDevice{ nullptr };
        CommandPool* m_pCommandPool{ nullptr };

        VkSwapchainKHR m_SwapChain{ VK_NULL_HANDLE };
        std::vector<Image> m_SwapChainImages{};
        VkFormat m_SwapChainImageFormat{ VK_FORMAT_UNDEFINED };
        VkExtent2D m_SwapChainExtent{};

        // Swapchain support cache
        struct SwapChainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities{};
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        } m_SwapChainSupport;

        // Sync objects per frame
        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence>     m_InFlightFences;

        PresentMode m_PresentMode{ PresentMode::FIFO };

        uint32_t m_CurrentFrame{ 0 };
    };
}
