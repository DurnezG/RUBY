#pragma once
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "Vulkan/Device.h"


namespace RUBY 
{
    class Shader
    {
    public:
        Shader() = default;
        Shader(Device* pDevice, const std::string& filePath, VkShaderStageFlagBits stage);

        Shader(const Shader& other) = delete;
        Shader(Shader&& other) noexcept;
        Shader& operator=(const Shader& other) = delete;
        Shader& operator=(Shader&& other) noexcept;

        ~Shader();

        VkShaderModule GetShaderModule() const { return m_ShaderModule; }
        VkPipelineShaderStageCreateInfo GetStageCreateInfo() const;

        static VkShaderModule CreateShaderModule(const VkDevice& logicalDevice, const std::vector<char>& code);
        static std::vector<char> ReadFile(const std::string& filePath);

    private:
        Device* m_pDevice{};
        VkShaderModule m_ShaderModule{};
        VkShaderStageFlagBits m_Stage{};
    };
}
