#pragma once
#include <vector>

#include <vulkan/vulkan_core.h>

#include "Vulkan/CommandPool.h"
#include "Vulkan/IBasePass.h"
#include "Vulkan/Device.h"
#include "Vulkan/SwapChain.h"

namespace RUBY
{
	class RUBY
	{
	public:
		RUBY(IRubyWindow* pWindow);
		~RUBY();

		void Render();

		bool BeginFrame(uint32_t& outImageIndex);
		void UpdateBuffers();
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void EndFrame(uint32_t imageIndex);
		void RecreateSwapChain();

	private:
		VkSampler CreateTextureSampler();

		void CreateSyncObjects();


		IRubyWindow* m_pWindow;

		Device m_Device{ m_pWindow };
		CommandPool m_CommandPool{ &m_Device };
		SwapChain m_SwapChain{ m_pWindow, &m_Device, &m_CommandPool };

		VkSampler m_TextureSampler{ CreateTextureSampler() };


		std::vector<IBasePass> m_Passes;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		bool m_FramebufferResized = false;
		uint32_t m_CurrentFrame = 0;

		int frameCount = 0;
	};
	
}
