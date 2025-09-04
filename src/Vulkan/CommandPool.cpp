#include "Vulkan/CommandPool.h"
#include "Vulkan/SwapChain.h"

#include <stdexcept>


RUBY::CommandPool::CommandPool(Device* pDevice)
	: m_pDevice(pDevice)
{
	CreateCommandPool();
	CreateCommandBuffers();
}

RUBY::CommandPool::~CommandPool()
{
	vkDestroyCommandPool(m_pDevice->GetLogicalDevice(), m_CommandPool, nullptr);
}

VkCommandBuffer RUBY::CommandPool::BeginSingleTimeCommands() const
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_pDevice->GetLogicalDevice(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void RUBY::CommandPool::EndSingleTimeCommands(VkCommandBuffer commandBuffer) const
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	VkFence fence;
	vkCreateFence(m_pDevice->GetLogicalDevice(), &fenceInfo, nullptr, &fence);

	vkQueueSubmit(m_pDevice->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkWaitForFences(m_pDevice->GetLogicalDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

	vkDestroyFence(m_pDevice->GetLogicalDevice(), fence, nullptr);
	//vkQueueWaitIdle(m_pDevice->GetGraphicsQueue());

	vkFreeCommandBuffers(m_pDevice->GetLogicalDevice(), m_CommandPool, 1, &commandBuffer);
}

void RUBY::CommandPool::CreateCommandPool()
{
	Device::QueueFamilyIndices queueFamilyIndices = m_pDevice->FindQueueFamilies(m_pDevice->GetPhysicalDevice());

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	VkResult result = vkCreateCommandPool(m_pDevice->GetLogicalDevice(), &poolInfo, nullptr, &m_CommandPool);
	if ( result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool (VkResult=" + std::to_string(result) + ")");
	}

	m_pDevice->GetDebugger().SetDebugName(
		reinterpret_cast<uint64_t>(m_CommandPool),
		"Main Command Pool",
		VK_OBJECT_TYPE_COMMAND_POOL
	);
}

void RUBY::CommandPool::CreateCommandBuffers()
{
	m_CommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	if (vkAllocateCommandBuffers(m_pDevice->GetLogicalDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}