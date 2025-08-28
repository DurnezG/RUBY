#pragma once
#include "Vulkan/Buffer.h"
#include "Vulkan/SwapChain.h"

namespace RUBY
{
	struct PassContext
	{
		Device* pDevice;
		CommandPool* pCommandPool;
		SwapChain* pSwapChain;
	};

	class IBasePass
	{
	public:
		virtual ~IBasePass() = default;
		virtual void CreateDescriptorSets() = 0;

		virtual void Update(uint32_t imageIndex) = 0;
		virtual void OnResize() = 0;

		virtual void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, PassContext& passContext) = 0;
	};
}


