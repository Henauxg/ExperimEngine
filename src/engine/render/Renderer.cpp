#include "Renderer.hpp"

#include <engine/EngineParameters.hpp>
#include <engine/log/ExpengineLog.hpp>

namespace expengine {
namespace render {

Renderer::Renderer(EngineParameters& engineParams)
    : engineParams_(engineParams)
    , logger_(spdlog::get(LOGGER_NAME)) {};

} // namespace render
} // namespace expengine
