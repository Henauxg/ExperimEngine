#pragma once

#include <cstdint>

namespace experim {

class Color {
public:
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color White;
    static const Color Black;
    static const Color Transparent;

public:
    Color(uint32_t rgba);
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    uint32_t toRGBA8Uint();

    uint8_t r_;
    uint8_t g_;
    uint8_t b_;
    uint8_t a_;
};

} // namespace experim
