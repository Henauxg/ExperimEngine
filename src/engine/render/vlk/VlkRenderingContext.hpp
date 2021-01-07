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
        const vlk::Device& device,
        std::shared_ptr<VulkanWindow> window,
        AttachmentsFlags attachmentFlags);
    ~VulkanRenderingContext() override;

    /* Accessors */
    inline const vk::SurfaceKHR surface() const { return windowSurface_.get(); };
    inline const Window& window() const override;

    /* Call to make the RenderingContext check its surface and adapt its objects to
     * it. */
    void handleSurfaceChanges() override;

    /* Frame rendering */
    void beginFrame() override;
    void submitFrame() override;
    /** Must not be called more than once per frame for now.
     * TODO : should have a common buffer interfaces between backends */
    vlk::FrameCommandBuffer& requestCommandBuffer();

    std::shared_ptr<RenderingContext> clone(
        std::shared_ptr<Window> window,
        AttachmentsFlags attachmentFlags) override;

private:
    struct FrameObjects {
        vk::UniqueCommandPool commandPool_;
        std::unique_ptr<vlk::FrameCommandBuffer> commandBuffer_;
        vk::UniqueFence fence_;
        vk::UniqueImageView imageView_;
        vk::UniqueFramebuffer framebuffer_;
    };

    struct FrameSemaphores {
        vk::UniqueSemaphore imageAcquired_;
        vk::UniqueSemaphore renderComplete_;
    };

    const vlk::Device& device_;

    /* Configuration */
    AttachmentsFlags attachmentsFlags_;

    /* Owned objects */
    std::shared_ptr<const vlk::VulkanWindow> window_;
    vk::UniqueSurfaceKHR windowSurface_;
    std::unique_ptr<vlk::Swapchain> vlkSwapchain_;
    vk::UniqueRenderPass renderPass_;
    vk::UniquePipeline uiGraphicsPipeline_;

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
    vk::UniquePipeline createGraphicsPipeline(
        const vlk::Device& device,
        vk::GraphicsPipelineCreateInfo& pipelineInfos,
        vk::RenderPass& renderPass);
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
    void buildSwapchainObjects(vk::Extent2D requestedExtent);
};

} // namespace vlk
} // namespace render
} // namespace expengine
