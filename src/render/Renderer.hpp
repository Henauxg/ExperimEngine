#pragma once

#include "EngineParameters.hpp"
#include "Window.hpp"
#include "vlk/VlkInclude.hpp"

namespace expengine {
namespace render {

class Renderer {
public:
	Renderer(const char* appName, const Window& window, EngineParameters& engineParams);
	~Renderer();

	void render();
	void rendererWaitIdle();

private:
	/**  @brief Given to the constructor. Used to create the rendering surface, access the needed
	 * extensions, query for resize, ... . */
	const Window& window_;
	EngineParameters& engineParams_;

	vk::UniqueInstance vkInstance_;
	/* TODO : vk::UniqueDebugUtilsMessengerEXT. See links in createDebugUtilsMessengerEXT
	 * implementation. */
	vk::DebugUtilsMessengerEXT vkDebugMessenger_;

	vk::UniqueInstance createVulkanInstance(const char* appName, const Window& window) const;
	vk::DebugUtilsMessengerEXT Renderer::setupDebugMessenger(vk::Instance instance,
															 bool enableValidationLayers) const;
};

} // namespace render
} // namespace expengine