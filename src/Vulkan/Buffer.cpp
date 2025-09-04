#include "Vulkan/Buffer.h"

#include <stdexcept>

//#define VMA_IMPLEMENTATION
#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

RUBY::Buffer::Buffer(Device* pDevice, CommandPool* pCommandPool, const VkBufferCreateInfo& bufferInfo, const VkMemoryPropertyFlags properties, HostAccess hostAcces)
	: m_pDevice(pDevice), m_pCommandPool(pCommandPool)
{
	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.requiredFlags = properties;

	switch (hostAcces)
	{
	case HostAccess::None:
		break;
	case HostAccess::Sequential:
		allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		break;
	case HostAccess::Random:
		allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		break;
	}

	m_Size = bufferInfo.size;

	vmaCreateBuffer(pDevice->GetAllocator(), &bufferInfo, &allocInfo, &m_Buffer, &m_BufferAllocation, nullptr);
	vmaSetAllocationName(pDevice->GetAllocator(), m_BufferAllocation, "MyBuffer");
}

RUBY::Buffer::~Buffer()
{
	if (m_Buffer != VK_NULL_HANDLE && m_BufferAllocation != VK_NULL_HANDLE)
		vmaDestroyBuffer(m_pDevice->GetAllocator(), m_Buffer, m_BufferAllocation);
}

RUBY::Buffer::Buffer(Buffer&& other) noexcept
{
	m_Buffer = other.m_Buffer;
	m_BufferAllocation = other.m_BufferAllocation;
	m_pDevice = other.m_pDevice;
	m_pCommandPool = other.m_pCommandPool;
	other.m_Buffer = VK_NULL_HANDLE;
	other.m_BufferAllocation = VK_NULL_HANDLE;
}

RUBY::Buffer& RUBY::Buffer::operator=(Buffer&& other) noexcept
{
	m_Buffer = other.m_Buffer;
	m_BufferAllocation = other.m_BufferAllocation;
	m_pDevice = other.m_pDevice;
	m_pCommandPool = other.m_pCommandPool;
	other.m_Buffer = VK_NULL_HANDLE;
	other.m_BufferAllocation = VK_NULL_HANDLE;

	return *this;
}

void RUBY::Buffer::CopyBuffer(VkBuffer srcBuffer, VkDeviceSize size) const
{
    VkCommandBuffer commandBuffer = m_pCommandPool->BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, m_Buffer, 1, &copyRegion);

    m_pCommandPool->EndSingleTimeCommands(commandBuffer);
}

void RUBY::Buffer::CopyMemory(void* data, const VkDeviceSize& size, int offset) const
{
	assert(offset + size <= m_Size && "CopyMemory out of bounds!");
	vmaCopyMemoryToAllocation(m_pDevice->GetAllocator(), data, m_BufferAllocation, offset, size);
}
