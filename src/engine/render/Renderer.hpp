#pragma once

#include <engine/EngineParameters.hpp>
#include <engine/log/ExpengineLog.hpp>
#include <engine/render/Window.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {

class Renderer {
public:
	Renderer(const char* appName, const Window& window,
			 EngineParameters& engineParams);
	~Renderer();

	void render();
	void rendererWaitIdle();

private:
	/**  @brief Given to the constructor. Used to create the rendering
	 * surface, access the needed extensions, query for resize, ... . */
	const Window& window_;
	EngineParameters& engineParams_;

	vk::UniqueInstance vkInstance_;
	vk::UniqueSurfaceKHR vkSurface_;
	vk::PhysicalDevice vkPhysicalDevice_;
	/**  @brief Only used in debug mode. */
	vk::DebugUtilsMessengerEXT
		vkDebugMessenger_; /* TODO : vk::UniqueDebugUtilsMessengerEXT */

	/* Logging */
	std::shared_ptr<spdlog::logger> logger_;

	vk::UniqueInstance createVulkanInstance(const char* appName,
											const Window& window) const;
	vk::DebugUtilsMessengerEXT
	Renderer::setupDebugMessenger(vk::Instance instance,
								  bool enableValidationLayers) const;
};

} // namespace render
} // namespace expengine