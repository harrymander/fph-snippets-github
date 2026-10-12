#pragma once

#include "portable-endian.h"

#include <bit>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace endian {

inline uint64_t htole(uint64_t v)
{
    return htole64(v);
}

inline uint32_t htole(uint32_t v)
{
    return htole32(v);
}

inline uint16_t htole(uint16_t v)
{
    return htole16(v);
}

inline uint8_t htole(uint8_t v)
{
    return v;
}

inline uint32_t letoh(uint32_t v)
{
    return le32toh(v);
}

inline uint16_t letoh(uint16_t v)
{
    return le16toh(v);
}

inline uint8_t letoh(uint8_t v)
{
    return v;
}

template <typename T> constexpr inline std::make_unsigned_t<T> unsigned_bit_cast(T val)
{
    static_assert(std::is_signed_v<T>, "T must be signed");
    return std::bit_cast<std::make_unsigned_t<T>>(val);
}

template <typename T> constexpr inline std::make_signed_t<T> signed_bit_cast(T val)
{
    static_assert(std::is_unsigned_v<T>, "T must be unsigned");
    return std::bit_cast<std::make_signed_t<T>>(val);
}

template <typename Signed>
inline std::enable_if_t<std::is_signed_v<Signed>, Signed> htole(Signed val)
{
    return std::bit_cast<Signed>(htole(unsigned_bit_cast(val)));
}

template <typename Signed>
inline std::enable_if_t<std::is_signed_v<Signed>, Signed> letoh(Signed val)
{
    return std::bit_cast<Signed>(letoh(unsigned_bit_cast(val)));
}

template <typename T>
inline std::enable_if_t<std::is_integral_v<T>> set_buffer_le(uint8_t *buffer, T val)
{
    T le_val = htole(val);
    std::memcpy(buffer, &le_val, sizeof(le_val));
}

template <typename T>
inline std::enable_if_t<std::is_integral_v<T>, T> get_buffer_le(const uint8_t *buffer)
{
    T le_val;
    std::memcpy(&le_val, buffer, sizeof(le_val));
    return letoh(le_val);
}

}; // namespace endian
