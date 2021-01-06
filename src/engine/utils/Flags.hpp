#pragma once

#ifndef __EMSCRIPTEN__

#include <compare>

#endif // !__EMSCRIPTEN__

namespace expengine {

template <typename FlagBitsType> struct FlagTraits {
    enum
    {
        allFlags = 0
    };
};

/* Initial version taken form vulkan.hpp */
template <typename BitType> class Flags {
public:
    using MaskType = typename std::underlying_type<BitType>::type;

    /* Constructors */
    constexpr Flags() noexcept
        : m_mask(0)
    {
    }

    constexpr Flags(BitType bit) noexcept
        : m_mask(static_cast<MaskType>(bit))
    {
    }

    constexpr Flags(Flags<BitType> const& rhs) noexcept
        : m_mask(rhs.m_mask)
    {
    }

    constexpr explicit Flags(MaskType flags) noexcept
        : m_mask(flags)
    {
    }

    /* Relational operators (c++20 spaceship operator)
     * https://devblogs.microsoft.com/cppblog/simplify-your-code-with-rocket-science-c20s-spaceship-operator/
     */
#ifndef __EMSCRIPTEN__
    auto operator<=>(Flags<BitType> const&) const = default;
#else
    constexpr bool operator<(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask < rhs.m_mask;
    }

    constexpr bool operator<=(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask <= rhs.m_mask;
    }

    constexpr bool operator>(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask > rhs.m_mask;
    }

    constexpr bool operator>=(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask >= rhs.m_mask;
    }

    constexpr bool operator==(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask == rhs.m_mask;
    }

    constexpr bool operator!=(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask != rhs.m_mask;
    }
#endif

    /* logical operator */
    constexpr bool operator!() const noexcept { return !m_mask; }

    /* Bitwise operators between flags */
    constexpr Flags<BitType> operator&(Flags<BitType> const& rhs) const noexcept
    {
        return Flags<BitType>(m_mask & rhs.m_mask);
    }

    constexpr Flags<BitType> operator|(Flags<BitType> const& rhs) const noexcept
    {
        return Flags<BitType>(m_mask | rhs.m_mask);
    }

    constexpr Flags<BitType> operator^(Flags<BitType> const& rhs) const noexcept
    {
        return Flags<BitType>(m_mask ^ rhs.m_mask);
    }

    constexpr Flags<BitType> operator~() const noexcept
    {
        return Flags<BitType>(m_mask ^ FlagTraits<BitType>::allFlags);
    }

    /* Assignment operators */
    constexpr Flags<BitType>& operator=(Flags<BitType> const& rhs) noexcept
    {
        m_mask = rhs.m_mask;
        return *this;
    }

    constexpr Flags<BitType>& operator|=(Flags<BitType> const& rhs) noexcept
    {
        m_mask |= rhs.m_mask;
        return *this;
    }

    constexpr Flags<BitType>& operator&=(Flags<BitType> const& rhs) noexcept
    {
        m_mask &= rhs.m_mask;
        return *this;
    }

    constexpr Flags<BitType>& operator^=(Flags<BitType> const& rhs) noexcept
    {
        m_mask ^= rhs.m_mask;
        return *this;
    }

    /* Cast operators */
    explicit constexpr operator bool() const noexcept { return !!m_mask; }

    explicit constexpr operator MaskType() const noexcept { return m_mask; }

private:
    MaskType m_mask;
};

/* Bitwise operators between flags and bit */
template <typename BitType>
constexpr Flags<BitType> operator&(BitType bit, Flags<BitType> const& flags) noexcept
{
    return flags & bit;
}

template <typename BitType>
constexpr Flags<BitType> operator|(BitType bit, Flags<BitType> const& flags) noexcept
{
    return flags | bit;
}

template <typename BitType>
constexpr Flags<BitType> operator^(BitType bit, Flags<BitType> const& flags) noexcept
{
    return flags ^ bit;
}
} // namespace expengine
