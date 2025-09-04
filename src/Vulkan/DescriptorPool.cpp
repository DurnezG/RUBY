#include "Vulkan/DescriptorPool.h"
#include <stdexcept>

RUBY::DescriptorPool::DescriptorPool(Device* pDevice,
    const std::vector<DescriptorSetLayoutData>& layoutDatas,
    const std::vector<VkDescriptorPoolSize>& poolSizes,
    uint32_t maxSets)
    : m_pDevice(pDevice)
{
    CreateDescriptorSetLayouts(layoutDatas);
    CreateDescriptorPool(poolSizes, maxSets);
}

RUBY::DescriptorPool::~DescriptorPool()
{
    if (m_DescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_pDevice->GetLogicalDevice(), m_DescriptorPool, nullptr);
    }

    for (auto layout : m_DescriptorSetLayouts)
    {
        vkDestroyDescriptorSetLayout(m_pDevice->GetLogicalDevice(), layout, nullptr);
    }
}

const VkDescriptorSetLayout& RUBY::DescriptorPool::GetDescriptorSetLayout(int index) const
{
    return m_DescriptorSetLayouts.at(index);
}

const std::vector<VkDescriptorSetLayout>& RUBY::DescriptorPool::GetDescriptorSetLayouts() const
{
    return m_DescriptorSetLayouts;
}

const VkDescriptorPool& RUBY::DescriptorPool::GetDescriptorPool() const
{
    return m_DescriptorPool;
}

void RUBY::DescriptorPool::CreateDescriptorSetLayouts(const std::vector<DescriptorSetLayoutData>& layoutDatas)
{
    m_DescriptorSetLayouts.resize(layoutDatas.size());

    for (size_t i = 0; i < layoutDatas.size(); ++i)
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutDatas[i].bindings.size());
        layoutInfo.pBindings = layoutDatas[i].bindings.data();

        if (vkCreateDescriptorSetLayout(
            m_pDevice->GetLogicalDevice(),
            &layoutInfo,
            nullptr,
            &m_DescriptorSetLayouts[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor set layout at index " + std::to_string(i));
        }
    }
}

void RUBY::DescriptorPool::CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets)
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxSets;

    if (vkCreateDescriptorPool(m_pDevice->GetLogicalDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}
