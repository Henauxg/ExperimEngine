#include "Renderer.hpp"

#include <stdexcept>

#include <ExperimEngineConfig.h>
#include <engine/render/imgui/impl/ImGuiBackend.hpp>
#include <engine/render/vlk/VlkCapabilities.hpp>
#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkDispatch.hpp>
#include <engine/render/vlk/VlkMemoryAllocator.hpp>

namespace {
const uint32_t ENGINE_VULKAN_API_VERSION = VK_API_VERSION_1_0;
}

namespace expengine {
namespace render {

Renderer::Renderer(
    const std::string& appName,
    const uint32_t appVersion,
    std::shared_ptr<Window> window,
    EngineParameters& engineParams)
    : mainWindow_(window)
    , engineParams_(engineParams)
    , logger_(spdlog::get(LOGGER_NAME))
{
    vk::DispatchLoaderDynamic& dispatchLoader_ = vlk::initializeDispatch();
    vkInstance_ = createVulkanInstance(appName, appVersion, *mainWindow_);
    vlk::initializeInstanceDispatch(*vkInstance_, dispatchLoader_);

    vkDebugMessenger_
        = setupDebugMessenger(*vkInstance_, vlk::ENABLE_VALIDATION_LAYERS);

    vlkDevice_ = std::make_unique<vlk::Device>(*vkInstance_, logger_);
    vlk::specializeDeviceDispatch(*vlkDevice_, dispatchLoader_);

    memAllocator_ = std::make_unique<vlk::MemoryAllocator>(
        *vkInstance_, *vlkDevice_, dispatchLoader_, ENGINE_VULKAN_API_VERSION);

    imguiBackend_ = std::make_unique<ImguiBackend>(*vlkDevice_, window);

    mainRenderingContext_ = std::make_shared<RenderingContext>(
        *vkInstance_,
        *vlkDevice_,
        window,
        imguiBackend_->getRenderingBackend(),
        AttachmentsFlagBits::eColorAttachment);

    imguiBackend_->bindMainRenderingContext(mainRenderingContext_);
}

Renderer::~Renderer()
{
    SPDLOG_LOGGER_DEBUG(logger_, "Renderer destruction");
    if (vlk::ENABLE_VALIDATION_LAYERS)
    {
        vlk::destroyDebugUtilsMessengerEXT(*vkInstance_, vkDebugMessenger_);
    }
}

void Renderer::render()
{ /* TODO Implement */
}

void Renderer::handleEvent(const SDL_Event& event)
{
    bool handled = imguiBackend_->handleEvent(event);

    if (!handled)
    {
        /* TODO handle rendering events */
    }
}

void Renderer::rendererWaitIdle() { vlkDevice_->waitIdle(); }

vk::UniqueInstance Renderer::createVulkanInstance(
    const std::string& appName,
    const uint32_t appVersion,
    const Window& window) const
{
    /* Check layer support */
    EXPENGINE_ASSERT(
        !vlk::ENABLE_VALIDATION_LAYERS
            || vlk::hasValidationLayerSupport(vlk::validationLayers),
        "Validation layer(s) requested, but not available.");

    /* Acquire all the required extensions */
    std::vector<const char*> extensions = window.getRequiredVkExtensions();
    if (vlk::ENABLE_VALIDATION_LAYERS)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    /* Check extension support */
    EXPENGINE_ASSERT(
        vlk::hasInstanceExtensionsSupport(extensions),
        "Vulkan extension(s) not supported.");

    /* Instance structs */
    vk::ApplicationInfo applicationInfo {
        .pApplicationName = appName.data(),
        .applicationVersion = appVersion,
        .pEngineName = EXPERIMENGINE_NAME,
        .engineVersion = VK_MAKE_VERSION(
            ExperimEngine_VERSION_MAJOR,
            ExperimEngine_VERSION_MINOR,
            ExperimEngine_VERSION_PATCH),
        .apiVersion = ENGINE_VULKAN_API_VERSION};

    vk::InstanceCreateInfo createInfo {
        .pApplicationInfo = &applicationInfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()};

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (vlk::ENABLE_VALIDATION_LAYERS)
    {
        createInfo.enabledLayerCount
            = static_cast<uint32_t>(vlk::validationLayers.size());
        createInfo.ppEnabledLayerNames = vlk::validationLayers.data();
        /* Give logger access to Debug Messenger */
        debugCreateInfo
            = vlk::getDebugUtilsCreateInfo(debugCreateInfo, logger_.get());
        createInfo.pNext = &debugCreateInfo;
    }

    auto [result, instance] = vk::createInstanceUnique(createInfo);
    EXPENGINE_VK_ASSERT(result, "Failed to create Vulkan instance.");

    return std::move(instance);
}

vk::DebugUtilsMessengerEXT Renderer::setupDebugMessenger(
    vk::Instance instance,
    bool enableValidationLayers) const
{
    if (!enableValidationLayers)
    {
        return nullptr;
    }

    return vlk::createDebugUtilsMessengerEXT(instance, logger_.get());
}

} // namespace render
} // namespace expengine
