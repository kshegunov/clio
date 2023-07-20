#pragma once

#include <type_traits>

#define CLIO_SERIALIZER(Name) \
    friend Clio::Serialization::Node<Name>; \
    friend Clio::Serialization::Object<Name>; \
    friend Clio::Serialization::Array<Name>; \
    using Clio::Serializer<Name>::value; \
    using Clio::Serializer<Name>::object;\
    using Clio::Serializer<Name>::array;

namespace Clio {
template <typename>
struct Serializer;
template <typename>
struct Deserializer;

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename = void, typename ... Args>
struct require_all : std::false_type {};
template <typename ... Args>
struct require_all<std::void_t<Args...>, Args...> : std::true_type {};
template <typename ... Args>
inline constexpr bool require_all_v = require_all<Args...>::value;

template <typename Type>
inline constexpr bool is_primitive_v = std::is_arithmetic_v<remove_cvref_t<Type>> || std::is_pointer_v<remove_cvref_t<Type>>;

template <typename Interface>
struct Node {
protected:
    constexpr Node(Interface& parent) : node(parent) {}
    Interface& node;
};
}
