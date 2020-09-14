#pragma once

#include <memory>
#include <vector>

#include <engine/render/Window.hpp>
#include <engine/render/vlk/VlkDevice.hpp>

namespace expengine {
namespace render {

struct FrameObjects {
	vk::CommandPool commandPool_;
	vk::CommandBuffer commandBuffer_;
	vk::Fence fence_;
	vk::Image backbuffer_;
	vk::ImageView backbufferView_;
	vk::Framebuffer framebuffer_;
	vk::Semaphore imageAcquiredSemaphore_;
	vk::Semaphore renderCompleteSemaphore_;
};

class RenderingContext {
public:
	RenderingContext(const vk::Instance vkInstance,
					 const vlk::Device& device,
					 std::shared_ptr<Window> window);
	~RenderingContext();

	/* Accessors */
	inline const vk::SurfaceKHR surface() { return windowSurface_.get(); };
	inline const Window& window() const { return *window_; };

	inline vk::CommandBuffer beginSingleTimeCommand() const
	{
		return device_.createTransientCommandBuffer();
	};
	inline void endSingleTimeCommand(vk::CommandBuffer commandBuffer) const
	{
		return device_.submitTransientCommandBuffer(commandBuffer);
	};

private:
	const vlk::Device& device_;

	std::shared_ptr<const Window> window_;
	vk::UniqueSurfaceKHR windowSurface_;
	/* Frames */
	uint32_t currentFrameIndex_;
	std::vector<FrameObjects> frames_;
};

} // namespace render
} // namespace expengine
