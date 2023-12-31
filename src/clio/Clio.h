// SPDX-FileCopyrightText: © 2023 Konstantin Shegunov <kshegunov@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#define CLIO_SERIALIZER(Name) \
    friend Clio::Serialization::Node<Name>; \
    friend Clio::Serialization::Object<Name>; \
    friend Clio::Serialization::Array<Name>; \
    friend Clio::Serialization::Blob<Name>; \
public:\
    using Clio::Serializer<Name>::value; \
    using Clio::Serializer<Name>::object; \
    using Clio::Serializer<Name>::array; \
  /*using Clio::Serializer<Name>::blob;*/

#define CLIO_DESERIALIZER(Name) \
    friend Clio::Deserialization::Node<Name>; \
    friend Clio::Deserialization::Object<Name>; \
    friend Clio::Deserialization::Array<Name>; \
    friend Clio::Deserialization::Blob<Name>; \
public:\
    using Clio::Deserializer<Name>::value; \
    using Clio::Deserializer<Name>::object; \
    using Clio::Deserializer<Name>::array; \
    using Clio::Deserializer<Name>::root; \
  /*using Clio::Deserializer<Name>::blob;*/

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

#ifndef __cpp_lib_remove_cvref
template <typename Type>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<Type>>;
#else
template <typename Type>
using remove_cvref_t = std::remove_cvref_t<Type>;
#endif

template <template <typename ...> class, typename ...>
struct is_instantiation_of : public std::false_type {};
template <template <typename ...> class Template, typename ... Arguments>
struct is_instantiation_of<Template, Template<Arguments ...>> : public std::true_type {};
template <template <typename ...> class Template, typename Type>
inline constexpr bool is_instantiation_of_v = is_instantiation_of<Template, remove_cvref_t<Type>>::value;
template <typename Head, typename ... Tail>
struct pack {};

template <typename Type>
inline constexpr bool is_serializer_v = std::is_base_of_v<Serializer<Type>, Type>;
template <typename Type>
inline constexpr bool is_deserializer_v = std::is_base_of_v<Deserializer<Type>, Type>;
template <typename Type>
inline constexpr bool is_primitive_v = std::is_arithmetic_v<remove_cvref_t<Type>> || std::is_pointer_v<remove_cvref_t<Type>> || std::is_array_v<remove_cvref_t<Type>>;

template <typename Interface>
struct Node {
protected:
    constexpr Node(Interface& parent) : node(parent) {}
    Interface& node;
};
}
