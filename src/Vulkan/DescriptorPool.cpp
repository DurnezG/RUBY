#include "Vulkan/DescriptorPool.h"

#include <array>

#include "Vulkan/Pipeline.h"
#include "Vulkan/SwapChain.h"

RUBY::DescriptorPool::DescriptorPool(Device* pDevice, const std::vector<VkDescriptorSetLayout>& layoutInfos)
	: m_pDevice(pDevice)
{
	CreateDescriptorSetLayouts(layoutInfos);
	//CreateDescriptorPool(poolInfo);
}

RUBY::DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(m_pDevice->GetLogicalDevice(), m_DescriptorPool, nullptr);

	for (int index{}; index < m_DescriptorSetLayouts.size(); ++index)
	{
		vkDestroyDescriptorSetLayout(m_pDevice->GetLogicalDevice(), m_DescriptorSetLayouts[index], nullptr);
	}
}

void RUBY::DescriptorPool::SetDescriptorSetPool(VkDescriptorPool poolInfo)
{
	CreateDescriptorPool(poolInfo);
}

const VkDescriptorSetLayout& RUBY::DescriptorPool::GetDescriptorSetLayout(int index) const
{
	return m_DescriptorSetLayouts[index];
}

const std::vector<VkDescriptorSetLayout>& RUBY::DescriptorPool::GetDescriptorSetLayouts() const
{
	return m_DescriptorSetLayouts;
}

const VkDescriptorPool& RUBY::DescriptorPool::GetDescriptorPool() const
{
	return m_DescriptorPool;
}

void RUBY::DescriptorPool::CreateDescriptorSetLayouts(const std::vector<VkDescriptorSetLayout>& layoutInfos)
{

	m_DescriptorSetLayouts = layoutInfos;
	//m_DescriptorSetLayouts.resize(layoutInfos.size());
	//for (int index{}; index < layoutInfos.size(); ++index)
	//{
	//	auto layoutInfo = layoutInfos[index];
	//	if (vkCreateDescriptorSetLayout(
	//		m_pDevice->GetLogicalDevice(),
	//		&layoutInfo.info,
	//		nullptr,
	//		&m_DescriptorSetLayouts[index]) != VK_SUCCESS)
	//	{
	//		throw std::runtime_error("Failed to create descriptor set layout" + index);
	//	}
	//}
}

void RUBY::DescriptorPool::CreateDescriptorPool(VkDescriptorPool poolInfo)
{
	m_DescriptorPool = poolInfo;
	//if (vkCreateDescriptorPool(
	//	m_pDevice->GetLogicalDevice(),
	//	&poolInfo,
	//	nullptr,
	//	&m_DescriptorPool) != VK_SUCCESS)
	//{
	//	throw std::runtime_error("failed to create descriptor pool!");
	//}
}
