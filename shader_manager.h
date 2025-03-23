#pragma once

#include <Volk/volk.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

namespace Simulator {
    class ShaderManager {
    public:
        ~ShaderManager();
        
        // Load a shader from compiled SPIR-V binary file
        bool loadShaderModule(
            const VkDevice& device,
            const std::string& filename,
            VkShaderModule& shaderModule,
            std::string& outErrorMessage);

        // Create shader from source code on-the-fly (requires shader compiler setup)
        bool createShaderModule(
            const VkDevice& device,
            const std::vector<uint32_t>& spirvCode,
            VkShaderModule& shaderModule,
            std::string& outErrorMessage);

        // Read a pre-compiled SPIR-V file
        static bool readShaderFile(
            const std::string& filename, 
            std::vector<uint32_t>& outSpirv,
            std::string& outErrorMessage);

        // Cleanup
        void destroy(const VkDevice& device);

    private:
        std::vector<VkShaderModule> m_shaderModules;
    };
}
