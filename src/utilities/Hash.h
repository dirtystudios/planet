#pragma once

#include <functional>

// this stuff isnt needed with c++14 it seems

// auto hash enums and fundamental types
//
////template <typename EnumType, typename RetType>
////using IsEnumRet = typename std::enable_if<std::is_enum<EnumType>::value, RetType>::type;
//
// template <typename Type, typename RetType>
// using IsHashableType = typename std::enable_if<std::is_class<std::hash<Type>>::value, RetType>::type;
//
//
// template <typename HashableType>
// IsHashableType<HashableType, size_t> Hash(const HashableType& e) {
//    return std::hash<HashableType>()(e);
//}
//
////template <typename EnumType>
////IsEnumRet<EnumType, size_t> Hash(const EnumType& e) {
////    using T = typename std::underlying_type<EnumType>::type;
////    return std::hash<T>()(static_cast<T>(e));
////}

inline void HashCombine(size_t& seed) {}

template <typename HashType, typename... HashArgs>
inline void HashCombine(size_t& seed, const HashType& v, HashArgs... args) {
    seed ^= std::hash<HashType>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    HashCombine(seed, args...);
}

template <typename HashType, typename... HashArgs>
inline size_t HashCombine(const HashType& v, HashArgs... args) {
    size_t key = 0;
    HashCombine(key, args...);
    return key;
}
