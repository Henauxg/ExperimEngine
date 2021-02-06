#pragma once

#include <memory>

#include <engine/render/RenderingContext.hpp>

namespace experim {

/** Abstract class. The renderer-specific derived class is stored in the void*
 * RenderUserData field of each ImGuiViewport to easily retrieve rendering backend
 * data. */
class ImGuiViewportRendererData {
public:
    virtual ~ImGuiViewportRendererData() = default;

    std::shared_ptr<RenderingContext> renderingContext_;

    virtual ImGuiViewportRendererData* clone(
        std::shared_ptr<RenderingContext> renderingContext)
        = 0;

protected:
    ImGuiViewportRendererData(std::shared_ptr<RenderingContext> renderingContext)
        : renderingContext_(renderingContext)
    {
        renderingContext->setSurfaceChangeCallback(
            std::bind(&ImGuiViewportRendererData::onSurfaceChange, this));
    }

    virtual void onSurfaceChange() = 0;
};

} // namespace experim
