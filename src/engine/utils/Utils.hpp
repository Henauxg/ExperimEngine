#pragma once

#include <cstdint>

#define EXPENGINE_MAKE_VERSION(major, minor, patch)                                 \
    ((((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

namespace expengine {

} // namespace expengine
