#include "Vulkan/Shader.h"

#include <fstream>
#include <stdexcept>


RUBY::Shader::Shader(Device* pDevice, const std::string& filePath, VkShaderStageFlagBits stage)
    : m_pDevice(pDevice), m_Stage(stage)
{
    m_ShaderModule = CreateShaderModule(pDevice->GetLogicalDevice(), ReadFile(filePath));
}


RUBY::Shader::Shader(Shader&& other) noexcept
{
    m_pDevice = other.m_pDevice;
    m_Stage = other.m_Stage;

    m_ShaderModule = other.m_ShaderModule;
    other.m_ShaderModule = VK_NULL_HANDLE;
    other.m_Stage = {};
}

RUBY::Shader& RUBY::Shader::operator=(Shader&& other) noexcept
{
    m_pDevice = other.m_pDevice;
    m_Stage = other.m_Stage;

    m_ShaderModule = other.m_ShaderModule;
    other.m_ShaderModule = VK_NULL_HANDLE;
    other.m_Stage = {};

    return *this;
}

RUBY::Shader::~Shader()
{
    if (m_ShaderModule != VK_NULL_HANDLE && m_pDevice != nullptr)
		vkDestroyShaderModule(m_pDevice->GetLogicalDevice(), m_ShaderModule, nullptr);
}

VkPipelineShaderStageCreateInfo RUBY::Shader::GetStageCreateInfo() const
{
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = m_Stage;
    stageInfo.module = m_ShaderModule;
    stageInfo.pName = "main";
    return stageInfo;
}

VkShaderModule RUBY::Shader::CreateShaderModule(const VkDevice& logicalDevice, const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

std::vector<char> RUBY::Shader::ReadFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open shader file: " + filePath);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
