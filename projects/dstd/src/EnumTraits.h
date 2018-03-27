#pragma once

#include <flags/flags.hpp>

// enum stuff

template <class E, class Enabler = void> struct is_enum_flags
: public std::false_type {};

// convert our "is enum flags" to flag modules "is enum flags"
template<typename E>
struct flags::is_flags<E, typename std::enable_if<is_enum_flags<E>::value>::type> : std::true_type {
};
