#include "VlkSwapchain.hpp"

#include <engine/render/vlk/VlkDebug.hpp>

namespace {

/* Preferred surface formats (from most to least) */
const std::vector<vk::SurfaceFormatKHR> SURFACE_FORMATS_PRIORITY_LIST
	= { { vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear },
		{ vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear },
		{ vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear },
		{ vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear },
		{ vk::Format::eB8G8R8A8Unorm,
		  vk::ColorSpaceKHR::eSrgbNonlinear } };

} // namespace

namespace expengine {
namespace render {
namespace vlk {

Swapchain::Swapchain(const vlk::Device& device, vk::SurfaceKHR& surface)
	: device_(device)
	, surface_(surface)
	, logger_(spdlog::get(LOGGER_NAME))
{
	/* Note: GPU has already been checked for WSI support when selected */
	/* Query the device swapchain properties for this surface */
	auto swapchainSupport = device.querySwapChainSupport(surface);

	/* Select the best Surface Format available */
	/* Simply request SURFACE_FORMATS_PRIORITY_LIST[0] as the preferred one
	 * for now */
	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
		SURFACE_FORMATS_PRIORITY_LIST[0], swapchainSupport.formats,
		SURFACE_FORMATS_PRIORITY_LIST);
	/* TODO Select the best Present Mode available */

	/* TODO Select the Swap Extent */

	/* TODO Create SwapChain */
}

/* Based on
 * https://github.com/KhronosGroup/Vulkan-Samples/blob/master/framework/core/swapchain.cpp
 */
vk::SurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(
	const vk::SurfaceFormatKHR requestedFormat,
	const std::vector<vk::SurfaceFormatKHR>& availableFormats,
	const std::vector<vk::SurfaceFormatKHR>& formatsPriorityList) const
{
	/* The best case scenario is that the surface has no preferred format,
	 * which Vulkan indicates by only returning one VkSurfaceFormatKHR
	 * entry which has its format member set to VK_FORMAT_UNDEFINED */
	if (availableFormats.size() == 1
		&& availableFormats[0].format == vk::Format::eUndefined)
	{
		SPDLOG_LOGGER_DEBUG(
			logger_, "Swapchain : surface format '{} ; {}' selected",
			requestedFormat.format, requestedFormat.colorSpace);
		return requestedFormat;
	}

	/* Try to find the requested format in the supported surface formats */
	auto surfaceFormatIterator = std::find_if(
		availableFormats.begin(), availableFormats.end(),
		[&requestedFormat](const vk::SurfaceFormatKHR& comparedFormat) {
			if (comparedFormat.format == requestedFormat.format
				&& comparedFormat.colorSpace == requestedFormat.colorSpace)
			{
				return true;
			}
			return false;
		});
	/* If the requested surface format isn't found, then try to request a
	 * format from the priority list */
	if (surfaceFormatIterator == availableFormats.end())
	{
		bool found = false;
		for (auto& surfaceFormat : formatsPriorityList)
		{
			surfaceFormatIterator = std::find_if(
				availableFormats.begin(), availableFormats.end(),
				[&surfaceFormat](
					const vk::SurfaceFormatKHR& comparedFormat) {
					if (comparedFormat.format == surfaceFormat.format
						&& comparedFormat.colorSpace
							== surfaceFormat.colorSpace)
					{
						return true;
					}
					return false;
				});
			if (surfaceFormatIterator != availableFormats.end())
			{
				found = true;
				break;
			}
		}

		/* If nothing is found, default to the first supported surface
		 * format */
		if (!found)
		{
			surfaceFormatIterator = availableFormats.begin();
		}
		SPDLOG_LOGGER_WARN(logger_,
						   "Swapchain : surface format '{} ; {}' not "
						   "supported, selecting '{} ; {}'",
						   requestedFormat.format,
						   requestedFormat.colorSpace,
						   (*surfaceFormatIterator).format,
						   (*surfaceFormatIterator).colorSpace);
	}
	else
	{
		/* The requested surface format was found */
		SPDLOG_LOGGER_DEBUG(
			logger_, "Swapchain : surface format '{} ; {}' selected",
			requestedFormat.format, requestedFormat.colorSpace);
	}

	return *surfaceFormatIterator;
}

} // namespace vlk
} // namespace render
} // namespace expengine
