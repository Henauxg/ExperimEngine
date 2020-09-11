#pragma once

#include <engine/EngineParameters.hpp>
#include <engine/log/ExpengineLog.hpp>
#include <engine/render/Window.hpp>
#include <engine/render/imgui/impl/ImguiBackend.hpp>
#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {

class Renderer {
public:
	Renderer(const char* appName, std::shared_ptr<Window> window,
			 EngineParameters& engineParams);
	~Renderer();

	void render();
	void rendererWaitIdle();

private:
	/**  @brief Given to the constructor. Used to create the rendering
	 * surface, access the needed extensions, query for resize, ... . */
	std::shared_ptr<const Window> mainWindow_;
	EngineParameters& engineParams_;

	/* Vulkan objects */
	vk::UniqueInstance vkInstance_;
	vk::UniqueSurfaceKHR vkMainWindowSurface_;
	std::unique_ptr<vlk::Device> vlkDevice_;
	vk::UniqueDescriptorPool vkDescriptorPool_;

	/* UI */
	std::unique_ptr<ImguiBackend> imguiBackend_;

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

	vk::UniqueDescriptorPool createDescriptorPool(vk::Device device) const;
};

} // namespace render
} // namespace expengine
