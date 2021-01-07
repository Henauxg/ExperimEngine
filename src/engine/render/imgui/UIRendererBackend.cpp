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

/* Helper structure stored in the void* RenderUserData field of each
 * ImGuiViewport to easily retrieve rendering backend data. */
struct ImGuiViewportRendererData {
    std::shared_ptr<RenderingContext> renderingContext_;

    ImGuiViewportRendererData(std::shared_ptr<RenderingContext> renderingContext)
        : renderingContext_(renderingContext)
    {
    }
};

/* Delegates */
static void ImGui_ImplExpengine_CreateWindow(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_SetWindowSize(ImGuiViewport* viewport, ImVec2 size);
static void ImGui_ImplExpengine_RenderWindow(
    ImGuiViewport* viewport,
    void* renderer_render_arg);
static void ImGui_ImplExpengine_SwapBuffers(ImGuiViewport* viewport, void*);

UIRendererBackend::UIRendererBackend(
    std::shared_ptr<ImGuiContextWrapper> context,
    std::shared_ptr<RenderingContext> mainRenderingContext,
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

    /* Setup main viewport RendererUserData */
    /* Cleaned by ImGui_ImplExpengine_DestroyWindow if viewport
     * enabled. Else cleaned by RendererBackend */
    ImGuiViewportRendererData* data
        = new ImGuiViewportRendererData(mainRenderingContext);
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    mainViewport->RendererUserData = data;
}

UIRendererBackend::~UIRendererBackend()
{
    SPDLOG_LOGGER_DEBUG(logger_, "UIRendererBackend destruction");
    /* Clean main viewport render data if viewport is not enabled */
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    if (ImGuiViewportRendererData* data
        = (ImGuiViewportRendererData*) mainViewport->RendererUserData)
        delete data;
    mainViewport->RendererUserData = nullptr;
}

static void ImGui_ImplExpengine_CreateWindow(ImGuiViewport* viewport)
{
    /* Get window from platform data */
    auto platformData = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    EXPENGINE_ASSERT(platformData != nullptr, "Error, null PlatformUserData");

    /* Use the main viewport RC to create a new renderer-specific RC */
    auto mainViewportData
        = (ImGuiViewportRendererData*) ImGui::GetMainViewport()->RendererUserData;
    EXPENGINE_ASSERT(
        mainViewportData != nullptr,
        "Error, null RendererUserData for main viewport");
    /* Create a RenderingContext. Surface creation is handled by the
     * RC. */
    auto renderingContext = mainViewportData->renderingContext_->clone(
        platformData->window_, AttachmentsFlagBits::eColorAttachment);

    /* Allocate RendererUserData */
    auto renderData = new ImGuiViewportRendererData(renderingContext);
    viewport->RendererUserData = renderData;
}

static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport)
{
    auto renderData = (ImGuiViewportRendererData*) viewport->RendererUserData;
    if (renderData)
    {
        delete renderData;
    }
    viewport->RendererUserData = nullptr;
}

static void ImGui_ImplExpengine_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ImGuiViewportRendererData* renderData
        = (ImGuiViewportRendererData*) viewport->RendererUserData;
    EXPENGINE_ASSERT(renderData != nullptr, "Error, null RendererUserData");

    /* TODO Here handle clear request : (viewport->Flags &
     * ImGuiViewportFlags_NoRendererClear) */

    /* Window size already set by platform backend. Simply notify the RC */
    renderData->renderingContext_->handleSurfaceChanges();
}

static void ImGui_ImplExpengine_RenderWindow(
    ImGuiViewport* viewport,
    void* renderer_render_arg)
{
    ImGuiViewportRendererData* renderData
        = (ImGuiViewportRendererData*) viewport->RendererUserData;
    EXPENGINE_ASSERT(renderData != nullptr, "Error, null RendererUserData");

    renderData->renderingContext_->beginFrame();

    auto uiRenderingBackend
        = reinterpret_cast<UIRendererBackend*>(renderer_render_arg);
    EXPENGINE_ASSERT(
        uiRenderingBackend != nullptr, "Error, null UI RenderingBackend");
    /* Setup state and record draw commands */
    uiRenderingBackend->renderUI(*renderData->renderingContext_, viewport->DrawData);
}

static void ImGui_ImplExpengine_SwapBuffers(ImGuiViewport* viewport, void*)
{
    ImGuiViewportRendererData* renderData
        = (ImGuiViewportRendererData*) viewport->RendererUserData;
    EXPENGINE_ASSERT(renderData != nullptr, "Error, null RendererUserData");

    /* Note : Queue submission for rendering is launched here instead of
     * ImGui_ImplExpengine_RenderWindow */
    renderData->renderingContext_->submitFrame();
}

} // namespace render
} // namespace expengine
