#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Device.h"

namespace RUBY
{
    class DescriptorPool
    {
    public:
        struct DescriptorSetLayoutData
        {
            std::vector<VkDescriptorSetLayoutBinding> bindings{};
        };

        DescriptorPool(Device* pDevice,
                       const std::vector<DescriptorSetLayoutData>& layoutDatas,
                       const std::vector<VkDescriptorPoolSize>& poolSizes,
                       uint32_t maxSets = MAX_POOL_RESERVE);

        DescriptorPool(const DescriptorPool& other) = delete;
        DescriptorPool(DescriptorPool&& other) noexcept = delete;
        DescriptorPool& operator=(const DescriptorPool& other) = delete;
        DescriptorPool& operator=(DescriptorPool&& other) noexcept = delete;

        ~DescriptorPool();

        const VkDescriptorSetLayout& GetDescriptorSetLayout(int index) const;
        const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const;
        const VkDescriptorPool& GetDescriptorPool() const;

        static constexpr uint32_t MAX_POOL_RESERVE = 512;

    private:
        void CreateDescriptorSetLayouts(const std::vector<DescriptorSetLayoutData>& layoutDatas);
        void CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets);

        Device* m_pDevice{};

        VkDescriptorPool m_DescriptorPool{VK_NULL_HANDLE};
        std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts{};
    };
}
