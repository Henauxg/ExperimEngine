#include "Renderer.hpp"

#include <engine/EngineParameters.hpp>
#include <engine/log/ExpengineLog.hpp>

namespace experim {

Renderer::Renderer(EngineParameters& engineParams)
    : engineParams_(engineParams)
    , logger_(spdlog::get(LOGGER_NAME)) {};

} // namespace experim
