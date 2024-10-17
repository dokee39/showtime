#pragma once

#include <string>
#include <type_traits>
#include <netdb.h>

namespace util {
double getTime();

/**socket**/
in_addr_t to_in_addr(const std::string &host);

/**iostream**/
template<typename T, typename = void>
struct is_streamable : std::false_type {};

template<typename T>
struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type {};
}

