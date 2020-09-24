#pragma once

#include <vector>

#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Swapchain {
public:
	Swapchain(const vlk::Device& device, vk::SurfaceKHR& surface,
			  vk::Extent2D requestedExtent);

private:
	/* References */
	const vlk::Device& device_;
	const vk::SurfaceKHR& surface_;

	vk::UniqueSwapchainKHR swapchain_;

	/* Logging */
	std::shared_ptr<spdlog::logger> logger_;

	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
		const vk::SurfaceFormatKHR requestedFormat,
		const std::vector<vk::SurfaceFormatKHR>& availableFormats,
		const std::vector<vk::SurfaceFormatKHR>& formatsPriorityList)
		const;
	vk::PresentModeKHR chooseSwapPresentMode(
		vk::PresentModeKHR requestedMode,
		const std::vector<vk::PresentModeKHR> availablePresentModes,
		const std::vector<vk::PresentModeKHR>& presentModePriorityList)
		const;
	vk::Extent2D chooseSwapExtent(vk::Extent2D requestedExtent,
								  vk::Extent2D currentExtent,
								  vk::Extent2D minImageExten,
								  vk::Extent2D maxImageExtent) const;
};
} // namespace vlk
} // namespace render
} // namespace expengine
