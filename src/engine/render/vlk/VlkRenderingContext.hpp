#pragma once

#include <memory>
#include <vector>

#include <engine/render/RenderingContext.hpp>
#include <engine/render/Window.hpp>
#include <engine/render/vlk/VlkInclude.hpp>
#include <engine/utils/Flags.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Swapchain;
class Device;
class FrameCommandBuffer;
class VulkanWindow;
class MemoryAllocator;
struct FrameObjects;

class VulkanRenderingContext : public RenderingContext {
public:
    VulkanRenderingContext(
        const Device& device,
        std::shared_ptr<VulkanWindow> window,
        AttachmentsFlags attachmentFlags,
        std::function<void(void)> surfaceChangeCallback = nullptr);
    ~VulkanRenderingContext() override;

    /* Accessors */
    inline const vk::SurfaceKHR surface() const { return windowSurface_.get(); };
    inline size_t imageCount() const { return frames_.size(); };
    inline const Window& window() const override;

    /** Call to make the RenderingContext check its surface and adapt its objects to
     * it. */
    void handleSurfaceChanges() override;
    /* Uses the implicit RC RenderPass */
    vk::UniquePipeline createGraphicsPipeline(
        vk::GraphicsPipelineCreateInfo& pipelineInfos);

    /* Frame rendering */
    void beginFrame() override;
    void submitFrame() override;
    /** Must not be called more than once per frame for now.
     * TODO : should have a common buffer interfaces between backends */
    vlk::FrameCommandBuffer& requestCommandBuffer();

    void waitIdle();

    std::shared_ptr<RenderingContext> clone(
        std::shared_ptr<Window> window,
        AttachmentsFlags attachmentFlags) override;

private:
    /* Types */
    struct FrameObjects {
        vk::UniqueFence fence_;
        vk::UniqueImageView imageView_;
        vk::UniqueFramebuffer framebuffer_;
        /* TODO Pool implementation/rework */
        vk::UniqueCommandPool commandPool_;
        std::vector<FrameCommandBuffer> commandBuffers_;
        std::vector<vk::CommandBuffer> commandBufferHandles_;
    };

    struct FrameSemaphores {
        vk::UniqueSemaphore imageAcquired_;
        vk::UniqueSemaphore renderComplete_;
    };

    /* References */
    const Device& device_;

    /* Configuration */
    AttachmentsFlags attachmentsFlags_;

    /* Owned objects */
    std::shared_ptr<const vlk::VulkanWindow> window_;
    vk::UniqueSurfaceKHR windowSurface_;
    std::unique_ptr<vlk::Swapchain> vlkSwapchain_;
    vk::UniqueRenderPass renderPass_;

    /* Frames */
    uint32_t frameIndex_;
    uint32_t semaphoreIndex_;
    std::vector<FrameObjects> frames_;
    std::vector<FrameSemaphores> semaphores_;
    /* Mapping to know which frame is using which semaphore group.
     * Semaphore Group ID -> Frame Fence
     * We can wait on the fence to make sure that the semaphore group is available */
    std::unordered_map<uint32_t, vk::Fence> semaphoreToFrameFence_;

    /* Objects creation */
    vk::UniqueRenderPass createRenderPass(
        const vlk::Device& device,
        const vlk::Swapchain& swapchain,
        AttachmentsFlags attachmentsFlags);
    void createFrameObjects(
        std::vector<FrameObjects>& frames,
        const vlk::Swapchain& swapchain,
        vk::RenderPass& renderPass,
        AttachmentsFlags attachmentsFlags);
    /* Called once at creation. Should also be called on each resize.
     * Reads the new image/surface size from the window directly.
     * Does the following :
     * [resize-only] Destroy previous resources except swapchain
     * Create SwapChain
     * [resize-only] Destroy old SwapChain
     * Create Render pass
     * Create Graphics pipeline(s) (using layout/shaders/info... from UI
     * backend)
     * Create Image views, Framebuffers, Sync objects, Command pools,
     * Command buffers */
    void buildSwapchainObjects(
        vk::Extent2D requestedExtent,
        vk::SwapchainKHR oldSwapchainHandle = nullptr);
};

} // namespace vlk
} // namespace render
} // namespace expengine
