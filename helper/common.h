#pragma once
#include <utility>
#include <iterator>

namespace Clio::detail {
template <typename Container, typename = void>
struct reserve {
    reserve(Container&, std::size_t) {}
};
template <typename Container>
struct reserve<Container, std::void_t<decltype(std::declval<Container>().reserve(std::size_t{}))>> {
    reserve(Container& c, std::size_t size) { c.reserve(size); }
};

template <typename Interface, typename Container>
void serialize_associative(Interface& s, const Container& v) {
    auto object = s.object();
    for (auto& [key, item] : v) {
        object.value(key, item);
    }
}

template <typename Interface, typename Container>
void deserialize_associative(Interface& d, Container& v) {
    using key_type = typename decltype(v)::key_type;
    using mapped_type = typename decltype(v)::mapped_type;

    auto object = d.object();
    auto size = object.size();
    reserve(v, size);
    for (std::size_t i = 0; i < size; ++i) {
        key_type key = key_type(object.keyAt(i));
        mapped_type value;
        object.value(std::as_const(key), value);
        v.emplace(std::move(key), std::move(value));
    }
}

template <typename Interface, typename Container>
void serialize_sequence(Interface& s, const Container& v) {
    auto array = s.array();
    for (auto& item : v) {
        array.value(item);
    }
}

template <typename Interface, typename Container>
void deserialize_sequence(Interface& d, Container& v) {
    auto array = d.array();
    auto size = array.size();
    reserve(v, size);
    auto inserter = std::inserter(v, v.end());
    for (std::size_t i = 0; i < size; ++i) {
        typename Container::value_type item;
        array.value(item);
        inserter = std::move(item);
    }
}
}
