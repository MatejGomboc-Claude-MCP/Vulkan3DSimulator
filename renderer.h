#pragma once

#include <Volk/volk.h>
#include <string>
#include <vector>
#include <array>
#include <mathgl.h>
#include "shader_manager.h"

namespace Simulator {
    // Structure for vertex data
    struct Vertex {
        mgl::vec3 position;
        mgl::vec3 color;
        
        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescription;
        }
        
        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
            
            // Position attribute
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, position);
            
            // Color attribute
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);
            
            return attributeDescriptions;
        }
    };

    class Renderer {
    public:
        ~Renderer();
        bool init(
            std::string& out_error_message, HINSTANCE app_instance, HWND window
#ifdef DEBUG
            , PFN_vkDebugUtilsMessengerCallbackEXT vulkan_debug_callback, void* vulkan_debug_callback_user_data
#endif
        );
        void destroy();
        bool getSupportedPhysicalDevices(std::vector<VkPhysicalDevice>& out_supported_devices, std::string& out_error_message);
        bool createLogicalDevice(const VkPhysicalDevice& physical_device, std::string& out_error_message);
        
        // New methods for triangle rendering
        bool setupTriangleRendering(std::string& out_error_message);
        bool render(std::string& out_error_message);

    private:
        static bool areDeviceExtensionsSupported(const VkPhysicalDevice& physical_device, const std::vector<const char*>& extensions, std::string& out_error_message);
        bool createSwapChain(std::string& out_error_message);
        bool createRenderPass(std::string& out_error_message);
        bool createGraphicsPipeline(std::string& out_error_message);
        bool createFramebuffers(std::string& out_error_message);
        bool createCommandPool(std::string& out_error_message);
        bool createVertexBuffer(std::string& out_error_message);
        bool createCommandBuffers(std::string& out_error_message);
        bool createSyncObjects(std::string& out_error_message);

#ifdef DEBUG
        static constexpr const char* const VK_LAYER_KHRONOS_VALIDATION_NAME = "VK_LAYER_KHRONOS_validation";
#endif

        // Basic Vulkan objects
        bool m_initialized = false;
        VkInstance m_vk_instance = VK_NULL_HANDLE;
#ifdef DEBUG
        VkDebugUtilsMessengerEXT m_vk_debug_messenger = VK_NULL_HANDLE;
#endif
        VkSurfaceKHR m_vk_surface = VK_NULL_HANDLE;
        VkDevice m_vk_logical_device = VK_NULL_HANDLE;
        
        // New members for triangle rendering
        VkQueue m_graphics_queue = VK_NULL_HANDLE;
        VkQueue m_present_queue = VK_NULL_HANDLE;
        uint32_t m_graphics_queue_family_idx = 0;
        uint32_t m_present_queue_family_idx = 0;
        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> m_swapchain_images;
        VkFormat m_swapchain_image_format = VK_FORMAT_UNDEFINED;
        VkExtent2D m_swapchain_extent = {0, 0};
        std::vector<VkImageView> m_swapchain_image_views;
        VkRenderPass m_render_pass = VK_NULL_HANDLE;
        VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
        VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> m_swapchain_framebuffers;
        VkCommandPool m_command_pool = VK_NULL_HANDLE;
        VkBuffer m_vertex_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_vertex_buffer_memory = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> m_command_buffers;
        
        // Synchronization objects
        VkSemaphore m_image_available_semaphore = VK_NULL_HANDLE;
        VkSemaphore m_render_finished_semaphore = VK_NULL_HANDLE;
        VkFence m_in_flight_fence = VK_NULL_HANDLE;
        
        // Shader manager
        ShaderManager m_shader_manager;
        
        // Triangle data
        std::vector<Vertex> m_triangle_vertices = {
            {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
        };
    };
}
