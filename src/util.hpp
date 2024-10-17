#pragma once

#include <iostream>
#include <type_traits>

namespace util {
double getTime();

/**iostream**/
template<typename T, typename = void>
struct is_streamable : std::false_type {};

template<typename T>
struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type {};
}

