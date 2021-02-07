#include "Color.hpp"

namespace experim {

const Color Color::Red = Color(255, 0, 0, 255);
const Color Color::Green = Color(0, 255, 0, 255);
const Color Color::Blue = Color(0, 0, 255, 255);
const Color Color::White = Color(255, 255, 255, 255);
const Color Color::Black = Color(0, 0, 0, 255);
const Color Color::Transparent = Color(0, 0, 0, 0);

Color::Color(uint32_t rgba)
{
    r_ = (rgba >> 24) & 0xFF;
    g_ = (rgba >> 16) & 0xFF;
    b_ = (rgba >> 8) & 0xFF;
    a_ = rgba & 0xFF;
}

Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    : r_(r)
    , g_(g)
    , b_(b)
    , a_(a)
{
}

uint32_t Color::toRGBA8Uint()
{
    uint32_t rgba = (r_ << 24) + (g_ << 16) + (b_ << 8) + a_;
    return rgba;
}

} // namespace experim
