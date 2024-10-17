#pragma once

#include <iostream>
#include <type_traits>
#include <functional>

namespace util {
double getTime();

/**iostream**/
template<typename T, typename = void>
struct is_streamable : std::false_type {};

template<typename T>
struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type {};
}

// TODO
/**hash for tuple**/
template <typename Tuple, std::size_t Index = std::tuple_size<Tuple>::value - 1>
struct TupleHasher {
    static std::size_t hash(const Tuple& tuple) {
        std::size_t h = TupleHasher<Tuple, Index - 1>::hash(tuple);
        h ^= std::hash<std::tuple_element_t<Index, Tuple>>{}(std::get<Index>(tuple)) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

template <typename Tuple>
struct TupleHasher<Tuple, 0> {
    static std::size_t hash(const Tuple& tuple) {
        return std::hash<std::tuple_element_t<0, Tuple>>{}(std::get<0>(tuple));
    }
};

template <typename... Args>
struct std::hash<std::tuple<Args...>> {
    std::size_t operator()(const std::tuple<Args...>& tuple) const {
        return TupleHasher<std::tuple<Args...>>::hash(tuple);
    }
};
