#pragma once

#include <memory>

namespace experim {

/* Forward declarations */
class Texture;
class Window;

/* Rendering subsystem interface */
class IRendering {
public:
    virtual std::unique_ptr<Texture> createTexture() = 0;
    virtual std::shared_ptr<Window> getMainWindow() const = 0;
};

} // namespace experim
