#pragma once

#include <memory>

namespace experim {

/* Forward declarations */
class Texture;

/* Rendering subsystem interface */
class IRendering {
public:
    virtual std::unique_ptr<Texture> createTexture() = 0;
};

} // namespace experim
