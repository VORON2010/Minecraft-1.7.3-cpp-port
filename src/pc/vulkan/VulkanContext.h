#include <functional>
#pragma once
#include <vulkan/vulkan.h>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>

class VulkanTexture;

class VulkanContext {
public:
    static VulkanContext& getInstance();

    void init(SDL_Window* window);
    void cleanup();

    VkCommandBuffer beginFrame();
    void endFrame(VkCommandBuffer commandBuffer);
    void pushMVP();
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    // One pipeline per fixed-function state combination, built on first use
    VkPipeline getPipeline(uint32_t key);
    std::vector<std::function<void()>> pendingTextureUploads;
    void flushTextureUploads();
    std::unordered_map<uint32_t, std::shared_ptr<VulkanTexture>> globalTextures;

    VkInstance getInstanceHandle() const { return instance; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkDevice getDevice() const { return device; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkSurfaceKHR getSurface() const { return surface; }
    VkSwapchainKHR getSwapchain() const { return swapchain; }
    VkRenderPass getRenderPass() const { return renderPass; }
    const std::vector<VkFramebuffer>& getFramebuffers() const { return swapchainFramebuffers; }
    // A resource may still be referenced by the last frame handed to the GPU.
    // Inside a frame that is the one being recorded (currentFrame); between
    // frames endFrame() has already advanced currentFrame, so it is the
    // previous slot. Queueing into currentFrame there freed the resource after
    // waiting on a fence three frames old -> GPU use-after-free.
    void deferDeletion(std::function<void()> func) {
        uint32_t slot = isFrameActive ? currentFrame : (currentFrame + MAX_FRAMES_IN_FLIGHT - 1) % MAX_FRAMES_IN_FLIGHT;
        deletionQueue[slot].push_back(std::move(func));
    }
    // Copies transient vertex data into this frame's streaming buffer.
    // Returns false when no frame is being recorded or allocation failed.
    bool streamVertices(const void* data, VkDeviceSize bytes, VkBuffer& outBuffer, VkDeviceSize& outOffset);
    VkExtent2D getSwapchainExtent() const { return swapchainExtent; }
    VkCommandPool getCommandPool() const { return commandPool; }
    VkCommandPool getUploadCommandPool() const { return uploadCommandPool; }
    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
    VkDescriptorPool getDescriptorPool() const { return descriptorPool; }
    VkCommandBuffer getCurrentCommandBuffer() const { return commandBuffers[currentFrame]; } bool getIsFrameActive() const { return isFrameActive; }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
    VulkanContext() = default;
    // globalTextures is declared before deletionQueue, so default member
    // destruction would run ~VulkanTexture -> deferDeletion on an already
    // destroyed queue at process exit. Drop the textures first.
    ~VulkanContext() { globalTextures.clear(); }

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamilyIndex = 0;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    std::vector<VkImageView> swapchainImageViews;
    VkFormat depthFormat;
    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapchainFramebuffers;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    std::unordered_map<uint32_t, VkPipeline> pipelineCache;
    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandPool uploadCommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    
    // Sync objects
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0; bool isFrameActive = false; 
    uint32_t imageIndex = 0;
    const int MAX_FRAMES_IN_FLIGHT = 3;
    std::vector<std::function<void()>> deletionQueue[3];

    // Per-frame persistently mapped vertex ring for immediate-mode draws.
    struct StreamBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        void* mapped = nullptr;
        VkDeviceSize capacity = 0;
        VkDeviceSize offset = 0;
    };
    StreamBuffer streamBuffers[3];
    bool createStreamBuffer(StreamBuffer& sb, VkDeviceSize capacity);
    void destroyStreamBuffer(StreamBuffer& sb);
    void createSurface(SDL_Window* window);
    void createSwapchain(SDL_Window* window);
    // Window resize / fullscreen toggle: rebuild everything sized to the window.
    // Returns false while the window has no area (minimized).
    bool recreateSwapchain();
    void destroySwapchainResources();
    SDL_Window* window = nullptr;
    bool swapchainDirty = false;
    int lastDrawableW = 0, lastDrawableH = 0;
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createDepthResources();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    VkShaderModule createShaderModule(const std::vector<char>& code);

};


