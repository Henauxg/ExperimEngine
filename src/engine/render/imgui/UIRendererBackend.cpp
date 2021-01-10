#include "UIRendererBackend.hpp"

#include <string>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/RenderingContext.hpp>
#include <engine/render/imgui/ImGuiContextWrapper.hpp>
#include <engine/render/imgui/ImGuiViewportPlatformData.hpp>

namespace {

} // namespace

namespace expengine {
namespace render {

/* Delegates */
static void ImGui_ImplExpengine_CreateWindow(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_SetWindowSize(ImGuiViewport* viewport, ImVec2 size);
static void ImGui_ImplExpengine_RenderWindow(
    ImGuiViewport* viewport,
    void* renderer_render_arg);
static void ImGui_ImplExpengine_SwapBuffers(ImGuiViewport*, void*);

UIRendererBackend::UIRendererBackend(
    std::shared_ptr<ImGuiContextWrapper> context,
    const std::string& renderingBackendName,
    bool hasVtxOffset,
    bool hasViewports)
    : imguiContext_(context)
    , logger_(spdlog::get(LOGGER_NAME))
{
    /* ------------------------------------------- */
    /* Setup Renderer capabilities flags           */
    /* ------------------------------------------- */

    /* Setup back-end capabilities flags */
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = renderingBackendName.c_str();
    /* We can honor the ImDrawCmd::VtxOffset field, allowing for large
     * meshes. */
    if (hasVtxOffset)
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    /* We can create multi-viewports on the Renderer side */
    if (hasViewports)
        io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

    /* ------------------------------------------- */
    /* Rendering bindings                          */
    /* ------------------------------------------- */

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        /* Bind rendering delegates */
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        /* No check for Platform_CreateVkSurface since handled by RC */
        platform_io.Renderer_CreateWindow = ImGui_ImplExpengine_CreateWindow;
        /* Note : also called on main viewport */
        platform_io.Renderer_DestroyWindow = ImGui_ImplExpengine_DestroyWindow;
        platform_io.Renderer_SetWindowSize = ImGui_ImplExpengine_SetWindowSize;
        platform_io.Renderer_RenderWindow = ImGui_ImplExpengine_RenderWindow;
        platform_io.Renderer_SwapBuffers = ImGui_ImplExpengine_SwapBuffers;
    }
}

UIRendererBackend::~UIRendererBackend()
{
    SPDLOG_LOGGER_DEBUG(logger_, "UIRendererBackend destruction");
    /* Clean main viewport render data if viewport is not enabled */
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    if (ImGuiViewportRendererData* viewportRendererData
        = (ImGuiViewportRendererData*) mainViewport->RendererUserData)
        delete viewportRendererData;
    mainViewport->RendererUserData = nullptr;
}

void UIRendererBackend::renderViewport(
    ImGuiViewport* viewport,
    ImGuiViewportRendererData* rendererData)
{
    /* Avoid rendering when minimized, scale coordinates for retina displays (screen
     * coordinates != framebuffer coordinates) */
    uint32_t fbWidth = static_cast<uint32_t>(
        viewport->DrawData->DisplaySize.x * viewport->DrawData->FramebufferScale.x);
    uint32_t fbHeight = static_cast<uint32_t>(
        viewport->DrawData->DisplaySize.y * viewport->DrawData->FramebufferScale.y);
    if (fbWidth != 0 && fbHeight != 0)
    {
        /* Setup state and record draw commands */
        uploadBuffersAndDraw(rendererData, viewport->DrawData, fbWidth, fbHeight);
    }
}

static void ImGui_ImplExpengine_CreateWindow(ImGuiViewport* viewport)
{
    /* Get window from platform data */
    auto platformData = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    EXPENGINE_ASSERT(platformData != nullptr, "Error, null PlatformUserData");

    /* Use the main viewport RC to create a new renderer-specific RC */
    auto mainViewportRendererData
        = (ImGuiViewportRendererData*) ImGui::GetMainViewport()->RendererUserData;
    EXPENGINE_ASSERT(
        mainViewportRendererData != nullptr,
        "Error, null RendererUserData for main viewport");
    /* Surface creation is handled by the RC. */
    auto renderingContext = mainViewportRendererData->renderingContext_->clone(
        platformData->window_, AttachmentsFlagBits::eColorAttachment);

    /* Allocate RendererUserData */
    auto viewportRendererData = mainViewportRendererData->clone(renderingContext);
    viewport->RendererUserData = viewportRendererData;
}

static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport)
{
    auto rendererData = (ImGuiViewportRendererData*) viewport->RendererUserData;
    if (rendererData)
    {
        delete rendererData;
    }
    viewport->RendererUserData = nullptr;
}

static void ImGui_ImplExpengine_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ImGuiViewportRendererData* rendererData
        = (ImGuiViewportRendererData*) viewport->RendererUserData;
    EXPENGINE_ASSERT(rendererData != nullptr, "Error, null RendererUserData");

    /* TODO Here handle clear request : (viewport->Flags &
     * ImGuiViewportFlags_NoRendererClear) */

    /* Window size already set by platform backend. Simply notify the RC */
    rendererData->renderingContext_->handleSurfaceChanges();
}

static void ImGui_ImplExpengine_RenderWindow(
    ImGuiViewport* viewport,
    void* renderer_render_arg)
{
    ImGuiViewportRendererData* rendererData
        = (ImGuiViewportRendererData*) viewport->RendererUserData;
    EXPENGINE_ASSERT(rendererData != nullptr, "Error, null RendererUserData");

    rendererData->renderingContext_->beginFrame();

    auto uiRenderingBackend
        = reinterpret_cast<UIRendererBackend*>(renderer_render_arg);
    EXPENGINE_ASSERT(
        uiRenderingBackend != nullptr, "Error, null UI RenderingBackend");

    uiRenderingBackend->renderViewport(viewport, rendererData);

    rendererData->renderingContext_->submitFrame();
}

static void ImGui_ImplExpengine_SwapBuffers(ImGuiViewport*, void*) { }

} // namespace render
} // namespace expengine
