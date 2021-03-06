#pragma once

#include <memory>

#include <engine/render/Window.hpp>

namespace experim {

/** Helper structure stored in the void* PlatformUserData field of each
 * ImGuiViewport to easily retrieve platfrom backend data. */
struct ImGuiViewportPlatformData {
    std::shared_ptr<Window> window_;

    ImGuiViewportPlatformData(std::shared_ptr<Window> window)
        : window_(window)
    {
    }
    ~ImGuiViewportPlatformData() { }
};

} // namespace experim
