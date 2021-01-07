#include "VlkRenderer.hpp"

#include <random>
#include <stdexcept>

#include <ExperimEngineConfig.h>
#include <engine/render/imgui/ImGuiBackend.hpp>
#include <engine/render/vlk/VlkCapabilities.hpp>
#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkDispatch.hpp>
#include <engine/render/vlk/VlkMemoryAllocator.hpp>
#include <engine/render/vlk/VlkRenderingContext.hpp>
#include <engine/render/vlk/VlkWindow.hpp>

namespace {
const uint32_t ENGINE_VULKAN_API_VERSION = VK_API_VERSION_1_0;
}

namespace expengine {
namespace render {
namespace vlk {

VulkanRenderer::VulkanRenderer(
    const std::string& appName,
    const uint32_t appVersion,
    int windowWidth,
    int windoHeight,
    EngineParameters& engineParams)
    : Renderer(engineParams)
{
    mainWindow_
        = std::make_shared<vlk::VulkanWindow>(windowWidth, windoHeight, appName);

    vk::DispatchLoaderDynamic& dispatchLoader_ = vlk::initializeDispatch();
    vkInstance_ = createVulkanInstance(appName, appVersion, *mainWindow_);
    vlk::initializeInstanceDispatch(*vkInstance_, dispatchLoader_);

    vkDebugMessenger_
        = setupDebugMessenger(*vkInstance_, vlk::ENABLE_VALIDATION_LAYERS);

    vlkDevice_ = std::make_unique<vlk::Device>(*vkInstance_, logger_);
    vlk::specializeDeviceDispatch(*vlkDevice_, dispatchLoader_);

    memAllocator_ = std::make_unique<vlk::MemoryAllocator>(
        *vkInstance_, *vlkDevice_, dispatchLoader_, ENGINE_VULKAN_API_VERSION);

    mainRenderingContext_ = std::make_shared<VulkanRenderingContext>(
        *vlkDevice_, mainWindow_, AttachmentsFlagBits::eColorAttachment);

    imguiBackend_
        = std::make_unique<ImguiBackend>(*this, mainRenderingContext_, mainWindow_);
}

VulkanRenderer::~VulkanRenderer()
{
    SPDLOG_LOGGER_DEBUG(logger_, "Vulkan renderer destruction");
    if (vlk::ENABLE_VALIDATION_LAYERS)
    {
        vlk::destroyDebugUtilsMessengerEXT(*vkInstance_, vkDebugMessenger_);
    }
}

void VulkanRenderer::render()
{
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(10, 17);
    std::this_thread::sleep_for(std::chrono::milliseconds(uni(rng)));
}

void VulkanRenderer::handleEvent(const SDL_Event& event)
{
    bool handled = imguiBackend_->handleEvent(event);

    if (!handled)
    {
        /* TODO handle rendering events */
    }
}

void VulkanRenderer::waitIdle() { vlkDevice_->waitIdle(); }

std::shared_ptr<Window> VulkanRenderer::getMainWindow()
{
    return std::static_pointer_cast<Window>(mainWindow_);
}

vk::UniqueInstance VulkanRenderer::createVulkanInstance(
    const std::string& appName,
    const uint32_t appVersion,
    const vlk::VulkanWindow& window) const
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

vk::DebugUtilsMessengerEXT VulkanRenderer::setupDebugMessenger(
    vk::Instance instance,
    bool enableValidationLayers) const
{
    if (!enableValidationLayers)
    {
        return nullptr;
    }

    return vlk::createDebugUtilsMessengerEXT(instance, logger_.get());
}

} // namespace vlk
} // namespace render
} // namespace expengine
