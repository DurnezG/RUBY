#pragma once
#include "CommandPool.h"
#include "Device.h"

class VmaAllocation_T;
using VmaAllocation = VmaAllocation_T*;

namespace RUBY
{

	enum class HostAccess { None, Sequential, Random };

	class Buffer
	{
	public:

		Buffer() = default;
		Buffer(Device* pDevice, CommandPool* pCommandPool, const VkBufferCreateInfo& bufferInfo, VkMemoryPropertyFlags properties, HostAccess hostAcces);
		~Buffer();

		Buffer(Buffer&) = delete;
		Buffer(Buffer&&) noexcept;
		Buffer& operator=(Buffer&) = delete;
		Buffer& operator=(Buffer&&) noexcept;

		VkBuffer GetBuffer() const { return m_Buffer; }
		VmaAllocation GetBufferAllocation() const { return m_BufferAllocation; }

		void CopyBuffer(VkBuffer srcBuffer, VkDeviceSize size) const;
		void CopyMemory(void* data, const VkDeviceSize& size, int offset = 0) const;

	private:
		Device* m_pDevice{};
		CommandPool* m_pCommandPool{};

		VkBuffer m_Buffer{};
		VmaAllocation m_BufferAllocation{};
		VkDeviceSize m_Size{ 0 };
	};
}
