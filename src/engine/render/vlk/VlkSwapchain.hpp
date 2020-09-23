#pragma once

#include <vector>

#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Swapchain {
public:
	Swapchain(const vlk::Device& device, vk::SurfaceKHR& surface);

private:
	/* References */
	const vlk::Device& device_;
	const vk::SurfaceKHR& surface_;

	/* Logging */
	std::shared_ptr<spdlog::logger> logger_;

	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
		const vk::SurfaceFormatKHR requestedFormat,
		const std::vector<vk::SurfaceFormatKHR>& availableFormats,
		const std::vector<vk::SurfaceFormatKHR>& formatsPriorityList)
		const;
	vk::PresentModeKHR chooseSwapPresentMode(
		const std::vector<vk::PresentModeKHR> availablePresentModes) const;
	vk::Extent2D
	chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;
};
} // namespace vlk
} // namespace render
} // namespace expengine
