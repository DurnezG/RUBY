#pragma once
#include <vector>

#include "Vulkan/CommandPool.h"
//#include "Vulkan/IBasePass.h"
#include "Vulkan/Device.h"
#include "Vulkan/SwapChain.h"

namespace RUBY
{
	class DemoPass;
	class RUBY
	{
	public:
		RUBY(IRubyWindow* pWindow);
		~RUBY();

		Device& GetDevice() { return m_Device; }
		SwapChain& GetSwapChain() { return m_SwapChain; }
		CommandPool& GetCommandPool() { return m_CommandPool; }

		uint32_t GetCurrentFrame() const { return m_CurrentFrame; }

		bool BeginFrame(uint32_t& outImageIndex);
		void RecordPasses(VkCommandBuffer& cmd, uint32_t& img);
		void EndFrame(uint32_t imageIndex);
		void Render();

		//void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	private:
		void RecreateSwapChain();


		//VkSampler CreateTextureSampler();

		IRubyWindow* m_pWindow;

		Device m_Device{ m_pWindow };
		CommandPool m_CommandPool{ &m_Device };
		SwapChain m_SwapChain{ m_pWindow, &m_Device, &m_CommandPool };

		std::unique_ptr<DemoPass> m_TrianglePass;

		bool m_FramebufferResized = false;
		uint32_t m_CurrentFrame = 0;

		int frameCount = 0;
	};
	
}
