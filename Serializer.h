#pragma once

#include <type_traits>
#include <utility>
#include <string_view>
#include <optional>

#define CLIO_SERIALIZER(Name) \
    friend Clio::Serialization::Node<Name>; \
    friend Clio::Serialization::Object<Name>; \
    friend Clio::Serialization::Array<Name>; \
    friend Clio::Serialization::traits<Name>; \
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

namespace Serialization {
template <typename Type>
using write_argument_t = std::conditional_t<is_primitive_v<Type>, remove_cvref_t<Type>, std::add_lvalue_reference_t<std::add_const_t<Type>>>;
template <typename Implementation>
struct traits {
    template <typename Type>
    using internal_write_prototype_t = void(remove_cvref_t<Implementation>::*)(write_argument_t<Type>);
    template <typename Type>
    using internal_write_trait = std::void_t<decltype(static_cast<internal_write_prototype_t<Type>>(&remove_cvref_t<Implementation>::write))>;
    template <typename, typename = void>
    struct has_internal_write : std::false_type {};
    template <typename Type>
    struct has_internal_write<Type, internal_write_trait<Type>> : std::true_type {};
    template <typename Type>
    inline static constexpr bool has_internal_write_v = has_internal_write<Type>::value;

    template <typename Type>
    using global_write_trait = std::void_t<
        decltype(serialize(std::declval<Implementation&>(), std::declval<const Type&>())),
        std::enable_if_t<!std::is_same_v<remove_cvref_t<Type>, write_argument_t<Type>>>
        >;
    template <typename, typename = void>
    struct has_global_write : std::false_type {};
    template <typename Type>
    struct has_global_write<Type, global_write_trait<Type>> : std::true_type {};
    template <typename Type>
    inline static constexpr bool has_global_write_v = has_global_write<Type>::value;
};

template <typename Interface>
struct Array;
template <typename Interface>
struct Object;

template <typename Interface>
struct Node : Clio::Node<Interface> {
    using Base = Clio::Node<Interface>;
    using Base::Base;

protected:
    using traits = Clio::Serialization::traits<Interface>;
    template <typename ValueType>
    void pushValue(ValueType v) {
        if constexpr (traits::template has_global_write_v<ValueType>) {
            serialize(this->node, std::forward<ValueType>(v));
        }
        else if constexpr (traits::template has_internal_write_v<ValueType>){
            this->node.write(std::forward<ValueType>(v));
        }
        else {
            cant_serialize(v);
        }
    }
};

template <typename Interface>
struct Object : Node<Interface> {
    Object(Interface& parent) : Node<Interface>(parent) {
        this->node.pushObject();
    }
    ~Object() {
        this->node.popObject();
    }

    template <typename Key>
    auto object(Key&& key) {
        this->node.pushKey(std::forward<Key>(key));
        return Object(this->node);
    }

    template <typename Key>
    auto array(Key&& key) {
        this->node.pushKey(std::forward<Key>(key));
        return Array(this->node);
    }
    template <typename Key, typename ValueType>
    std::enable_if_t<is_primitive_v<ValueType>> value(Key&& key, const ValueType v) {
        this->node.pushKey(std::forward<Key>(key));
        this->pushValue(v);
    }
    template <typename Key, typename ValueType>
    std::enable_if_t<!is_primitive_v<ValueType>> value(Key&& key, const ValueType& v) {
        this->node.pushKey(std::forward<Key>(key));
        this->pushValue(v);
    }
    template <typename Key, typename ValueType>
    void value(Key&& key, const std::optional<ValueType>& v) {
        if (!v) return;
        value(std::forward<Key>(key), v.value());
    }
};

template <typename Interface>
struct Array : Node<Interface> {
    Array(Interface& parent) : Node<Interface>(parent) {
        this->node.pushArray();
    }
    ~Array() {
        this->node.pushArray();
    }

    auto object() { return Object(this->node); }
    auto array() { return Array(this->node); }

    template <typename ValueType>
    std::enable_if_t<is_primitive_v<ValueType>> value(const ValueType v) {
        this->pushValue(v);
    }

    template <typename ValueType>
    std::enable_if_t<!is_primitive_v<ValueType>> value(const ValueType& v) {
        this->pushValue(v);
    }

    template <typename ValueType>
    void value(const std::optional<ValueType>& v) {
        if (!v) return;
        value(v.value());
    }
};
}

template <typename Interface>
struct Serializer : Serialization::Node<Interface> {
    Serializer() : Serialization::Node<Interface>(static_cast<Interface&>(*this)) {}

    auto object() { return Serialization::Object(this->node); }
    auto array() { return Serialization::Array(this->node); }

    template <typename ValueType>
    std::enable_if_t<Clio::is_primitive_v<ValueType>> value(const ValueType v) {
        this->pushValue(v);
    }
    template <typename ValueType>
    std::enable_if_t<!Clio::is_primitive_v<ValueType>> value(const ValueType& v) {
        this->pushValue(v);
    }
};
}
