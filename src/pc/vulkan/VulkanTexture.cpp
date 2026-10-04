#include <thread>
#include "pc/OpenGL.h"
#include "VulkanTexture.h"
#include "VulkanContext.h"
#include <stdexcept>
#include <cstring>

VulkanTexture::VulkanTexture() {}

VulkanTexture::~VulkanTexture() {
    cleanup();
}

void VulkanTexture::cleanup() {
    VkDevice device = VulkanContext::getInstance().getDevice();
    if (device == VK_NULL_HANDLE) return;

    // Replacing a texture id (skin download, texture pack reload) drops the
    // old VulkanTexture while frames that sample it may still be in flight.
    // Destroying it right away is a GPU use-after-free, so hand the handles to
    // the deletion queue instead.
    VkSampler oldSampler = sampler;
    VkImageView oldView = imageView;
    VkImage oldImage = image;
    VkDeviceMemory oldMemory = imageMemory;
    VkDescriptorSet oldSet = descriptorSet;
    VkDescriptorPool pool = VulkanContext::getInstance().getDescriptorPool();
    sampler = VK_NULL_HANDLE;
    imageView = VK_NULL_HANDLE;
    image = VK_NULL_HANDLE;
    imageMemory = VK_NULL_HANDLE;
    descriptorSet = VK_NULL_HANDLE;
    if (!oldSampler && !oldView && !oldImage && !oldMemory && !oldSet) return;

    VulkanContext::getInstance().deferDeletion([device, pool, oldSampler, oldView, oldImage, oldMemory, oldSet]() {
        if (oldSet != VK_NULL_HANDLE) vkFreeDescriptorSets(device, pool, 1, &oldSet);
        if (oldSampler != VK_NULL_HANDLE) vkDestroySampler(device, oldSampler, nullptr);
        if (oldView != VK_NULL_HANDLE) vkDestroyImageView(device, oldView, nullptr);
        if (oldImage != VK_NULL_HANDLE) vkDestroyImage(device, oldImage, nullptr);
        if (oldMemory != VK_NULL_HANDLE) vkFreeMemory(device, oldMemory, nullptr);
    });
}

uint32_t VulkanTexture::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDevice physicalDevice = VulkanContext::getInstance().getPhysicalDevice();
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanTexture::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkDevice device = VulkanContext::getInstance().getDevice();
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

VkCommandBuffer VulkanTexture::beginSingleTimeCommands() {
    VkDevice device = VulkanContext::getInstance().getDevice();
    VkCommandPool commandPool = VulkanContext::getInstance().getUploadCommandPool();

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkResult res = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    if (res != VK_SUCCESS) {
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult res2 = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (res2 != VK_SUCCESS) {
    }
    return commandBuffer;
}

void VulkanTexture::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    VkDevice device = VulkanContext::getInstance().getDevice();
    VkCommandPool commandPool = VulkanContext::getInstance().getUploadCommandPool();
    VkQueue graphicsQueue = VulkanContext::getInstance().getGraphicsQueue();

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkResult subRes = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE); 
    VkResult res = vkQueueWaitIdle(graphicsQueue); 

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void VulkanTexture::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage img, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = img;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    
}

void VulkanTexture::copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage img, uint32_t width, uint32_t height) {
    

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        img,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    
}

void VulkanTexture::upload(int w, int h, const void* pixels) {
    this->width = w;
    this->height = h;
    cpuPixels.assign(static_cast<const uint8_t*>(pixels), static_cast<const uint8_t*>(pixels) + static_cast<size_t>(w) * h * 4);
    dirtyX0 = dirtyY0 = dirtyX1 = dirtyY1 = 0;

    VkDeviceSize imageSize = w * h * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    VkDevice device = VulkanContext::getInstance().getDevice();
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);

    VkCommandBuffer cmd = beginSingleTimeCommands();
    transitionImageLayout(cmd, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(cmd, stagingBuffer, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    transitionImageLayout(cmd, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    endSingleTimeCommands(cmd);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    VkFilter filter = blurMode ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    VkSamplerAddressMode address = clampMode ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.magFilter = filter;
    samplerInfo.minFilter = filter;
    samplerInfo.addressModeU = address;
    samplerInfo.addressModeV = address;
    samplerInfo.addressModeW = address;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    VkDescriptorSetAllocateInfo allocInfoSet{};
    allocInfoSet.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfoSet.descriptorPool = VulkanContext::getInstance().getDescriptorPool();
    VkDescriptorSetLayout layouts[] = {VulkanContext::getInstance().getDescriptorSetLayout()};
    allocInfoSet.descriptorSetCount = 1;
    allocInfoSet.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(device, &allocInfoSet, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    VkDescriptorImageInfo imageInfoSet{};
    imageInfoSet.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoSet.imageView = imageView;
    imageInfoSet.sampler = sampler;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfoSet;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

bool VulkanTexture::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t set) {
    if (descriptorSet == VK_NULL_HANDLE) return false;
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, set, 1, &descriptorSet, 0, nullptr);
    return true;
}














void VulkanTexture::updateSub(int x, int y, int w, int h, const uint8_t* pixels) {
    if (cpuPixels.empty() || pixels == nullptr) return;
    int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
    int x1 = x + w > width ? width : x + w, y1 = y + h > height ? height : y + h;
    if (x1 <= x0 || y1 <= y0) return;

    for (int row = y0; row < y1; row++) {
        const uint8_t* src = pixels + (static_cast<size_t>(row - y) * w + (x0 - x)) * 4;
        uint8_t* dst = cpuPixels.data() + (static_cast<size_t>(row) * width + x0) * 4;
        std::memcpy(dst, src, static_cast<size_t>(x1 - x0) * 4);
    }

    if (dirtyX1 <= dirtyX0) {
        dirtyX0 = x0; dirtyY0 = y0; dirtyX1 = x1; dirtyY1 = y1;
    } else {
        if (x0 < dirtyX0) dirtyX0 = x0;
        if (y0 < dirtyY0) dirtyY0 = y0;
        if (x1 > dirtyX1) dirtyX1 = x1;
        if (y1 > dirtyY1) dirtyY1 = y1;
    }
}

void VulkanTexture::recordPendingUpdate(VkCommandBuffer cmd) {
    if (!hasPendingUpdate()) return;
    int x0 = dirtyX0, y0 = dirtyY0, w = dirtyX1 - dirtyX0, h = dirtyY1 - dirtyY0;

    std::vector<uint8_t> rect(static_cast<size_t>(w) * h * 4);
    for (int row = 0; row < h; row++) {
        std::memcpy(rect.data() + static_cast<size_t>(row) * w * 4,
                    cpuPixels.data() + (static_cast<size_t>(y0 + row) * width + x0) * 4,
                    static_cast<size_t>(w) * 4);
    }

    VkBuffer staging = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    if (!VulkanContext::getInstance().streamVertices(rect.data(), rect.size(), staging, offset)) return;
    dirtyX0 = dirtyY0 = dirtyX1 = dirtyY1 = 0;

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    // Earlier frames may still sample the image: wait for their fragment work
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkBufferImageCopy region{};
    region.bufferOffset = offset;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {x0, y0, 0};
    region.imageExtent = {static_cast<uint32_t>(w), static_cast<uint32_t>(h), 1};
    vkCmdCopyBufferToImage(cmd, staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}
