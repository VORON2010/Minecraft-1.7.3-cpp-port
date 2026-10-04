#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <cstdint>

class VulkanTexture {
public:
    VulkanTexture();
    ~VulkanTexture();

    void upload(int width, int height, const void* pixels);
    bool bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t set);
    void cleanup();
    // %clamp% / %blur% textures, call before upload()
    void setSamplerMode(bool clamp, bool blur) { clampMode = clamp; blurMode = blur; }

    // glTexSubImage2D: patches the CPU copy, the GPU image is updated by
    // recordPendingUpdate at the start of the next frame
    void updateSub(int x, int y, int w, int h, const uint8_t* pixels);
    bool hasPendingUpdate() const { return dirtyX1 > dirtyX0 && image != VK_NULL_HANDLE; }
    // Must be recorded outside a render pass
    void recordPendingUpdate(VkCommandBuffer cmd);
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    VkImageView getImageView() const { return imageView; }
    VkSampler getSampler() const { return sampler; }

private:
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    bool clampMode = false;
    bool blurMode = false;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    
    int width = 0;
    int height = 0;

    std::vector<uint8_t> cpuPixels;
    int dirtyX0 = 0, dirtyY0 = 0, dirtyX1 = 0, dirtyY1 = 0;
};

