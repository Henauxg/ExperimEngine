#pragma once

#include <memory>
#include <vector>

#include <engine/render/Window.hpp>
#include <engine/render/imgui/impl/UIRendererBackendVulkan.hpp>
#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkSwapchain.hpp>
#include <engine/utils/Flags.hpp>

namespace expengine {
namespace render {

enum class AttachmentsFlagBits : uint32_t
{
    eColorAttachment = 1,
    eDepthAttachments = 1 << 1
};
using AttachmentsFlags = Flags<AttachmentsFlagBits>;

struct FrameObjects;

class RenderingContext {
public:
    RenderingContext(
        const vk::Instance vkInstance,
        const vlk::Device& device,
        std::shared_ptr<Window> window,
        const UIRendererBackendVulkan& imguiRenderBackend,
        AttachmentsFlags attachmentFlags);
    ~RenderingContext();

    /* Accessors */
    inline const vk::SurfaceKHR surface() const { return windowSurface_.get(); };
    inline const Window& window() const { return *window_; };

    inline vk::CommandBuffer beginSingleTimeCommand() const
    {
        return device_.createTransientCommandBuffer();
    };
    inline void endSingleTimeCommand(vk::CommandBuffer commandBuffer) const
    {
        return device_.submitTransientCommandBuffer(commandBuffer);
    };

    void handleSurfaceChanges();

private:
    struct FrameObjects {
        vk::UniqueCommandPool commandPool_;
        vk::UniqueCommandBuffer commandBuffer_;
        vk::UniqueFence fence_;
        vk::UniqueImageView imageView_;
        vk::UniqueFramebuffer framebuffer_;
    };

    struct FrameSemaphores {
        vk::UniqueSemaphore imageAcquired_;
        vk::UniqueSemaphore renderComplete_;
    };

    const vlk::Device& device_;
    const UIRendererBackendVulkan& imguiRenderBackend_;

    /* Configuration */
    AttachmentsFlags attachmentsFlags_;

    /* Owned objects */
    std::shared_ptr<const Window> window_;
    vk::UniqueSurfaceKHR windowSurface_;
    std::unique_ptr<vlk::Swapchain> vlkSwapchain_;
    vk::UniqueRenderPass renderPass_;
    vk::UniquePipeline uiGraphicsPipeline_;

    /* Frames */
    uint32_t frameIndex_;
    uint32_t semaphoreIndex_;
    std::vector<FrameObjects> frames_;
    std::vector<FrameSemaphores> semaphores_;
    std::unordered_map<uint32_t, vk::Fence> semaphoreToFrameFence_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;

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
    void initOrResizeRenderingContext();

    /* Frame rendering */
    vk::CommandBuffer& beginFrame();
    void submitFrame(const vk::CommandBuffer& cmdBuffer);
};

} // namespace render
} // namespace expengine
