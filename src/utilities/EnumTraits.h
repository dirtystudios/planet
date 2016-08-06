#pragma once

// enum stuff

template<typename EnumType>
struct EnumTraits {};

template <typename EnumType, typename RetType>
using IsBitflagEnumRet = typename std::enable_if<EnumTraits<EnumType>::is_bitflags, RetType>::type;

template<typename EnumType>
constexpr IsBitflagEnumRet<EnumType, bool> EnumBitflagTrue(EnumType e) {
    using T = typename std::underlying_type<EnumType>::type;
    return static_cast<T>(e) != 0;
}

template<typename EnumType>
constexpr IsBitflagEnumRet<EnumType, EnumType> operator&(EnumType a, EnumType b) {
    using T = typename std::underlying_type<EnumType>::type;
    return static_cast<EnumType>(static_cast<T>(a) & static_cast<T>(b));
}

template<typename EnumType>
constexpr IsBitflagEnumRet<EnumType, EnumType> operator|(EnumType a, EnumType b) {
    using T = typename std::underlying_type<EnumType>::type;
    return static_cast<EnumType>(static_cast<T>(a) | static_cast<T>(b));
}

template<typename EnumType>
constexpr IsBitflagEnumRet<EnumType, EnumType> operator^(EnumType a, EnumType b) {
    using T = typename std::underlying_type<EnumType>::type;
    return static_cast<EnumType>(static_cast<T>(a) ^ static_cast<T>(b));
}
