#pragma once

#include <type_traits>

#define CLIO_SERIALIZER(Name) \
    friend Clio::Serialization::Node<Name>; \
    friend Clio::Serialization::Object<Name>; \
    friend Clio::Serialization::Array<Name>; \
    friend Clio::Serialization::Blob<Name>; \
    using Clio::Serializer<Name>::value; \
    using Clio::Serializer<Name>::object; \
    using Clio::Serializer<Name>::array; \
    using Clio::Serializer<Name>::blob;

#define CLIO_DESERIALIZER(Name) \
    friend Clio::Deserialization::Node<Name>; \
    friend Clio::Deserialization::Object<Name>; \
    friend Clio::Deserialization::Array<Name>; \
    friend Clio::Deserialization::Blob<Name>; \
    using Clio::Deserialization<Name>::value; \
    using Clio::Deserialization<Name>::object; \
    using Clio::Deserialization<Name>::array; \
    using Clio::Deserialization<Name>::blob;

namespace Clio {
template <typename>
struct Serializer;
namespace Serialization {
template <typename Interface>
struct Array;
template <typename Interface>
struct Object;
template <typename Interface>
struct Blob;
}
template <typename>
struct Deserializer;
namespace Deserialization {
template <typename Interface>
struct Array;
template <typename Interface>
struct Object;
template <typename Interface>
struct Blob;
}

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename Type>
inline constexpr bool is_serializer_v = std::is_base_of_v<Serializer<Type>, Type>;
template <typename Type>
inline constexpr bool is_deserializer_v = std::is_base_of_v<Deserializer<Type>, Type>;
template <typename Type>
inline constexpr bool is_primitive_v = std::is_arithmetic_v<remove_cvref_t<Type>> || std::is_pointer_v<remove_cvref_t<Type>>;

template <typename Interface>
struct Node {
protected:
    constexpr Node(Interface& parent) : node(parent) {}
    Interface& node;
};
}
