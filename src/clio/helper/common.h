// SPDX-FileCopyrightText: © 2023 Konstantin Shegunov <kshegunov@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once
#include <utility>
#include <iterator>
#include <functional>

namespace Clio::detail {
template <typename Container, typename = void>
struct reserve {
    reserve(Container&, std::size_t) {}
};
template <typename Container>
struct reserve<Container, std::void_t<decltype(std::declval<Container>().reserve(std::size_t{}))>> {
    reserve(Container& c, std::size_t size) {
        c.clear();
        c.reserve(size);
    }
};

template <typename Functor, typename Interface, typename Key, typename Item, typename = void>
struct is_extended_functor : std::false_type {};
template <typename Functor, typename Interface, typename Key, typename Item>
struct is_extended_functor<Functor, Interface, Key, Item, std::void_t<decltype(
    std::declval<Functor>()(std::declval<Interface&>(), std::declval<Key>(), std::declval<Item&>())
)>> : std::true_type {};
template <typename Functor, typename Interface, typename Key, typename Item>
inline constexpr bool is_extended_functor_v = is_extended_functor<Functor, Interface, Key, Item>::value;

template <typename Functor, typename Interface, typename ... Arguments>
inline constexpr bool is_functor = std::is_invocable_v<Functor, Interface&, Arguments...>;

// -- Sequence helpers (a.k.a. std::vector, std::set, std::unordered_set) --

template <typename Interface, typename Container>
void serialize_sequence(Interface& s, const Container& v) {
    auto array = s.array();
    for (auto& item : v) {
        array.value(item);
    }
}

template <typename Interface, typename Container>
void deserialize_sequence(Interface& d, Container& v) {
    using Size = decltype(std::size(v));
    using Item = typename Container::value_type;

    auto array = d.array();
    reserve(v, array.size());
    auto inserter = std::inserter(v, v.end());
    for (Size i = 0, size = array.size(); i < size; ++i) {
        Item item;
        array.value(item);
        inserter = std::move(item);
    }
}

template <typename Interface, typename Container, typename Head, typename ... Tail>
void serialize_sequence(Interface& s, const Container& v, Head&& head, Tail&& ... args) {
    using Size = decltype(std::size(v));
    using Item = std::add_const_t<typename Container::value_type>;

    auto array = s.array();
    if constexpr (is_functor<Head, Interface, Item, Tail...>) {
        auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, std::placeholders::_2, std::forward<Tail>(args)...);
        for (auto& item : v) {
            array.value(item, f);
        }
    }
    else if constexpr (is_functor<Head, Interface, Size, Item, Tail...>) {
        Size i = 0;
        for (auto& item : v) {
            auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, i++, std::placeholders::_2, std::forward<Tail>(args)...);
            array.value(item, std::move(f));
        }
    }
    else {
        for (auto& item : v) {
            array.value(item, std::forward<Head>(head), std::forward<Tail>(args)...);
        }
    }
}

template <typename Interface, typename Container, typename Head, typename ... Tail>
void deserialize_sequence(Interface& d, Container& v, Head&& head, Tail&& ... args) {
    using Size = decltype(std::size(v));
    using Item = typename Container::value_type;

    auto array = d.array();
    reserve(v, array.size());
    auto inserter = std::inserter(v, v.end());

    if constexpr (is_functor<Head, Interface, Item&, Tail...>) {
        auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, std::placeholders::_2, std::forward<Tail>(args)...);
        for (Size i = 0, size = array.size(); i < size; ++i) {
            Item item;
            array.value(item, f);
            inserter = std::move(item);
        }
    }
    else if constexpr (is_functor<Head, Interface, Size, Item&, Tail...>) {
        for (Size i = 0, size = array.size(); i < size; ++i) {
            Item item;
            auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, i, std::placeholders::_2, std::forward<Tail>(args)...);
            array.value(item, std::move(f));
            inserter = std::move(item);
        }
    }
    else {
        for (Size i = 0, size = array.size(); i < size; ++i) {
            Item item;
            array.value(item, std::forward<Head>(head), std::forward<Tail>(args)...);
            inserter = std::move(item);
        }
    }
}

// -- Associative maps helpers (a.k.a. std::map, std::unordered_map, or other map-like classes)

template <typename Interface, typename Container>
void serialize_associative_direct(Interface& s, const Container& v) {
    auto object = s.object();
    for (auto& [key, item] : v) {
        object.value(key, item);
    }
}

template <typename Interface, typename Container>
void deserialize_associative_direct(Interface& d, Container& v) {
    using Size = decltype(d.object().size());
    using Key = typename Container::key_type;
    using Item = typename Container::mapped_type;

    auto object = d.object();
    reserve(v, object.size());
    for (Size i = 0, size = object.size(); i < size; ++i) {
        Key key = Key(object.peekKey());
        Item value;
        object.value(key, value);
        v.emplace(std::move(key), std::move(value));
    }
}

template <typename Interface, typename Container, typename Head, typename ... Tail>
void serialize_associative_direct(Interface& s, const Container& v, Head&& head, Tail&& ... args) {
    using Key = std::add_const_t<typename Container::key_type>;
    using Item = typename Container::mapped_type;

    auto object = s.object();
    if constexpr (is_functor<Head, Interface, Item, Tail...>) {
        auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, std::placeholders::_2, std::forward<Tail>(args)...);
        for (auto& [key, item] : v) {
            object.value(key, item, f);
        }
    }
    else if constexpr (is_functor<Head, Interface, Key, Item, Tail...>) {
        for (auto& [key, item] : v) {
            auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, std::as_const(key), std::placeholders::_2, std::forward<Tail>(args)...);
            object.value(key, item, std::move(f));
        }
    }
    else {
        for (auto& [key, item] : v) {
            object.value(key, item, std::forward<Head>(head), std::forward<Tail>(args)...);
        }
    }
}

template <typename Interface, typename Container, typename Head, typename ... Tail>
void deserialize_associative_direct(Interface& d, Container& v, Head&& head, Tail&& ... args) {
    using Size = decltype(d.object().size());
    using Key = typename Container::key_type;
    using Item = typename Container::mapped_type;

    auto object = d.object();
    reserve(v, object.size());
    if constexpr (is_functor<Head, Interface, Item&, Tail...>) {
        auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, std::placeholders::_2, std::forward<Tail>(args)...);
        for (Size i = 0, size = object.size(); i < size; ++i) {
            Key key = Key(object.peekKey());
            Item item;
            object.value(key, item, f);
            v.emplace(std::move(key), std::move(item));
        }
    }
    else if constexpr (is_functor<Head, Interface, std::add_const_t<Key>&, Item&, Tail...>) {
        for (Size i = 0, size = object.size(); i < size; ++i) {
            Key key = Key(object.peekKey());
            Item item;
            auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, std::as_const(key), std::placeholders::_2, std::forward<Tail>(args)...);
            object.value(key, item, std::move(f));
            v.emplace(std::move(key), std::move(item));
        }
    }
    else {
        for (Size i = 0, size = object.size(); i < size; ++i) {
            Key key = Key(object.peekKey());
            Item item;
            object.value(key, item, std::forward<Head>(head), std::forward<Tail>(args)...);
            v.emplace(std::move(key), std::move(item));
        }
    }
}

constexpr auto key_label = "key";
constexpr auto value_label = "value";

template <typename Interface, typename Container>
void serialize_associative_generic(Interface& s, const Container& v) {
    auto array = s.array();
    for (auto& [key, item] : v) {
        auto object = array.object();
        object.value(key_label, key);
        object.value(value_label, item);
    }
}

template <typename Interface, typename Container>
void deserialize_associative_generic(Interface& d, Container& v) {
    using Size = decltype(d.array().size());
    using Key = typename Container::key_type;
    using Item = typename Container::mapped_type;

    auto array = d.array();
    reserve(v, array.size());
    for (Size i = 0, size = array.size(); i < size; ++i) {
        Key key;
        Item value;
        auto object = d.object();
        object.value(key_label, key);
        object.value(value_label, value);
        v.emplace(std::move(key), std::move(value));
    }
}

template <typename Interface, typename Container, typename Head, typename ... Tail>
void serialize_associative_generic(Interface& s, const Container& v, Head&& head, Tail&& ... args) {
    using Key = std::add_const_t<typename Container::key_type>;
    using Item = typename Container::mapped_type;

    auto array = s.array();
    if constexpr (is_functor<Head, Interface, Item, Tail...>) {
        auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, std::placeholders::_2, std::forward<Tail>(args)...);
        for (auto& [key, item] : v) {
            auto object = array.object();
            object.value(key_label, key);
            object.value(value_label, item, f);
        }
    }
    else if constexpr (is_functor<Head, Interface, Key, Item, Tail...>) {
        for (auto& [key, item] : v) {
            auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, key, std::placeholders::_2, std::forward<Tail>(args)...);
            array.value(item, std::move(f));
        }
    }
    else {
        for (auto& [key, item] : v) {
            auto object = array.object();
            object.value(key_label, key);
            object.value(value_label, item, std::forward<Head>(head), std::forward<Tail>(args)...);
        }
    }
}

template <typename Interface, typename Container, typename Head, typename ... Tail>
void deserialize_associative_generic(Interface& d, Container& v, Head&& head, Tail&& ... args) {
    using Size = decltype(d.array().size());
    using Key = typename Container::key_type;
    using Item = typename Container::mapped_type;

    auto array = d.array();
    reserve(v, array.size());
    if constexpr (is_functor<Head, Interface, Item&, Tail...>) {
        auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, std::placeholders::_2, std::forward<Tail>(args)...);
        for (Size i = 0, size = array.size(); i < size; ++i) {
            Key key;
            Item item;
            auto object = array.object();
            object.value(key_label, key);
            object.value(value_label, item, f);
            v.emplace(std::move(key), std::move(item));
        }
    }
    else if constexpr (is_functor<Head, Interface, Key&, Item&, Tail...>) {
        for (Size i = 0, size = array.size(); i < size; ++i) {
            Key key;
            Item item;
            auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, std::ref(key), std::placeholders::_2, std::forward<Tail>(args)...);
            array.value(item, std::move(f));
            v.emplace(std::move(key), std::move(item));
        }
    }
    else {
        for (Size i = 0, size = array.size(); i < size; ++i) {
            Key key;
            Item item;
            auto object = array.object();
            object.value(key_label, key);
            object.value(value_label, item, std::forward<Head>(head), std::forward<Tail>(args)...);
            v.emplace(std::move(key), std::move(item));
        }
    }
}

template <typename Interface, typename Container, typename ... Arguments>
void serialize_associative(Interface& s, const Container& v, Arguments&& ... args) {
    using Key = typename Container::key_type;
    if constexpr (std::is_convertible_v<Key, std::string_view>) {
        serialize_associative_direct(s, v, std::forward<Arguments>(args)...);
    }
    else {
        serialize_associative_generic(s, v, std::forward<Arguments>(args)...);
    }
}

template <typename Interface, typename Container, typename ... Arguments>
void deserialize_associative(Interface& s, Container& v, Arguments&& ... args) {
    using Key = typename Container::key_type;
    if constexpr (std::is_convertible_v<Key, std::string_view>) {
        deserialize_associative_direct(s, v, std::forward<Arguments>(args)...);
    }
    else {
        deserialize_associative_generic(s, v, std::forward<Arguments>(args)...);
    }
}

// -- Fixed-size sequence helpers (a.k.a. std::array, Type[], etc.)

template <typename Interface, typename Container, typename ... Arguments>
void serialize_fixed_sequence(Interface& s, const Container& v, Arguments&& ... args) { serialize_sequence(s, v, std::forward<Arguments>(args)...); }

template <typename Interface, typename Container>
void deserialize_fixed_sequence(Interface& d, Container& v) {
    using Size = decltype(std::size(v));
    auto array = d.array();
    if (array.size() != std::size(v)) throw std::runtime_error("Fixed-size array size mismatch: expecting " + std::to_string(std::size(v)) + ", got " + std::to_string(array.size()));
    for (Size i = 0, size = array.size(); i < size; ++i) {
        array.value(v[i]);
    }
}

template <typename Interface, typename Container, typename Head, typename ... Tail>
void deserialize_fixed_sequence(Interface& d, Container& v, Head&& head, Tail&& ... args) {
    using Size = decltype(std::size(v));
    using Item = std::remove_reference_t<decltype(*std::begin(v))>;

    auto array = d.array();
    if (array.size() != std::size(v)) throw std::runtime_error("Fixed-size array size mismatch: expecting " + std::to_string(std::size(v)) + ", got " + std::to_string(array.size()));
    if constexpr (is_functor<Head, Interface, Item&, Tail...>) {
        auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, std::placeholders::_2, std::forward<Tail>(args)...);
        for (Size i = 0, size = array.size(); i < size; ++i) {
            array.value(v[i], f);
        }
    }
    else if constexpr (is_functor<Head, Interface, Size, Item&, Tail...>) {
        for (Size i = 0, size = array.size(); i < size; ++i) {
            auto f = std::bind(std::forward<Head>(head), std::placeholders::_1, i, std::placeholders::_2, std::forward<Tail>(args)...);
            array.value(v[i], std::move(f));
        }
    }
    else {
        for (Size i = 0, size = array.size(); i < size; ++i) {
            array.value(v[i], std::forward<Head>(head), std::forward<Tail>(args)...);
        }
    }
}
}
