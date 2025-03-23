// Continuing from previous file contents...

bool Renderer::createCommandBuffers(std::string& out_error_message) {
    m_command_buffers.resize(m_swapchain_framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_command_buffers.size());

    VkResult result = vkAllocateCommandBuffers(m_vk_logical_device, &allocInfo, m_command_buffers.data());
    if (result != VK_SUCCESS) {
        out_error_message = "Failed to allocate command buffers. VK error:" + std::to_string(result);
        return false;
    }

    // Record command buffers for triangle rendering
    for (size_t i = 0; i < m_command_buffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        result = vkBeginCommandBuffer(m_command_buffers[i], &beginInfo);
        if (result != VK_SUCCESS) {
            out_error_message = "Failed to begin recording command buffer. VK error:" + std::to_string(result);
            return false;
        }

        // Begin render pass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_render_pass;
        renderPassInfo.framebuffer = m_swapchain_framebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchain_extent;

        // Clear color (background color)
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.2f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        // Begin render pass
        vkCmdBeginRenderPass(m_command_buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Bind the graphics pipeline
        vkCmdBindPipeline(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

        // Bind vertex buffer
        VkBuffer vertexBuffers[] = {m_vertex_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(m_command_buffers[i], 0, 1, vertexBuffers, offsets);

        // Draw command
        vkCmdDraw(m_command_buffers[i], static_cast<uint32_t>(m_triangle_vertices.size()), 1, 0, 0);

        // End render pass
        vkCmdEndRenderPass(m_command_buffers[i]);

        // End command buffer
        result = vkEndCommandBuffer(m_command_buffers[i]);
        if (result != VK_SUCCESS) {
            out_error_message = "Failed to record command buffer. VK error:" + std::to_string(result);
            return false;
        }
    }

    return true;
}

bool Renderer::createSyncObjects(std::string& out_error_message) {
    // Create semaphores and fence
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start in signaled state so first wait will proceed

    VkResult result = vkCreateSemaphore(m_vk_logical_device, &semaphoreInfo, nullptr, &m_image_available_semaphore);
    if (result != VK_SUCCESS) {
        out_error_message = "Failed to create image available semaphore. VK error:" + std::to_string(result);
        return false;
    }

    result = vkCreateSemaphore(m_vk_logical_device, &semaphoreInfo, nullptr, &m_render_finished_semaphore);
    if (result != VK_SUCCESS) {
        out_error_message = "Failed to create render finished semaphore. VK error:" + std::to_string(result);
        return false;
    }

    result = vkCreateFence(m_vk_logical_device, &fenceInfo, nullptr, &m_in_flight_fence);
    if (result != VK_SUCCESS) {
        out_error_message = "Failed to create in-flight fence. VK error:" + std::to_string(result);
        return false;
    }

    return true;
}

bool Renderer::render(std::string& out_error_message) {
    // Wait for the previous frame to finish
    vkWaitForFences(m_vk_logical_device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_vk_logical_device, 1, &m_in_flight_fence);

    // Acquire an image from the swap chain
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_vk_logical_device, m_swapchain, UINT64_MAX, m_image_available_semaphore, VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Recreate the swap chain
        std::string recreateError;
        createSwapChain(recreateError);
        createFramebuffers(recreateError);
        return true;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        out_error_message = "Failed to acquire swap chain image. VK error:" + std::to_string(result);
        return false;
    }

    // Submit the command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_image_available_semaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_command_buffers[imageIndex];

    VkSemaphore signalSemaphores[] = {m_render_finished_semaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    result = vkQueueSubmit(m_graphics_queue, 1, &submitInfo, m_in_flight_fence);
    if (result != VK_SUCCESS) {
        out_error_message = "Failed to submit draw command buffer. VK error:" + std::to_string(result);
        return false;
    }

    // Present the image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_present_queue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Recreate the swap chain
        std::string recreateError;
        createSwapChain(recreateError);
        createFramebuffers(recreateError);
    } else if (result != VK_SUCCESS) {
        out_error_message = "Failed to present swap chain image. VK error:" + std::to_string(result);
        return false;
    }

    return true;
}
