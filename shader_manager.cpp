#include "shader_manager.h"

using namespace Simulator;

ShaderManager::~ShaderManager() {
    // Destructor doesn't destroy shader modules as it needs the device
    // Call destroy(device) explicitly before destroying the manager
}

bool ShaderManager::loadShaderModule(
    const VkDevice& device,
    const std::string& filename,
    VkShaderModule& shaderModule,
    std::string& outErrorMessage) {
    
    std::vector<uint32_t> spirvCode;
    if (!readShaderFile(filename, spirvCode, outErrorMessage)) {
        return false;
    }
    
    return createShaderModule(device, spirvCode, shaderModule, outErrorMessage);
}

bool ShaderManager::createShaderModule(
    const VkDevice& device,
    const std::vector<uint32_t>& spirvCode,
    VkShaderModule& shaderModule,
    std::string& outErrorMessage) {
    
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
    createInfo.pCode = spirvCode.data();
    
    VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS) {
        outErrorMessage = "Failed to create shader module. VK error:" + std::to_string(result) + ".";
        return false;
    }
    
    m_shaderModules.push_back(shaderModule);
    return true;
}

bool ShaderManager::readShaderFile(
    const std::string& filename,
    std::vector<uint32_t>& outSpirv,
    std::string& outErrorMessage) {
    
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        outErrorMessage = "Failed to open shader file: " + filename;
        return false;
    }
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    if (fileSize % sizeof(uint32_t) != 0) {
        outErrorMessage = "Shader file size is not a multiple of 4 bytes (SPIR-V requirement): " + filename;
        return false;
    }
    
    outSpirv.resize(fileSize / sizeof(uint32_t));
    
    file.seekg(0);
    file.read(reinterpret_cast<char*>(outSpirv.data()), fileSize);
    file.close();
    
    return true;
}

void ShaderManager::destroy(const VkDevice& device) {
    for (auto& module : m_shaderModules) {
        if (module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device, module, nullptr);
            module = VK_NULL_HANDLE;
        }
    }
    m_shaderModules.clear();
}
