// SPDX-FileCopyrightText: © 2023 Konstantin Shegunov <kshegunov@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once
#include "Clio.h"
#include <utility>
#include <optional>

namespace Clio::Serialization {
template <typename Interface>
struct Node : Clio::Node<Interface> {
    using Base = Clio::Node<Interface>;
    using Base::Base;

protected:
    template <typename ValueType>
    std::enable_if_t<is_primitive_v<ValueType>> value(ValueType v) {
        if constexpr (has_internal_write<ValueType>()) {
            this->node.write(v);
        }
        else {
            cant_serialize(v);
        }
    }

    template <typename ValueType, typename Functor>
    std::enable_if_t<is_primitive_v<ValueType>> value(ValueType v, Functor&& f) {
        if constexpr (std::is_invocable_v<Functor, Interface&, ValueType>) {
            std::forward<Functor>(f)(this->node, v);
        }
        else {
            cant_serialize(v);
        }
    }

    template <typename ValueType>
    std::enable_if_t<!is_primitive_v<ValueType>> value(const ValueType& v) {
        if constexpr (has_global_write<ValueType>()) {
            serialize(this->node, v);
        }
        else if constexpr (has_internal_write<ValueType>()) {
            this->node.write(v);
        }
        else {
            cant_serialize(v);
        }
    }

    template <typename ValueType, typename Head, typename ... Tail>
    std::enable_if_t<!is_primitive_v<ValueType>> value(const ValueType& v, Head&& head, Tail&& ... tail) {
        if constexpr (std::is_invocable_v<Head, Interface&, const ValueType&, Tail...>) {
            std::forward<Head>(head)(this->node, v, std::forward<Tail>(tail)...);
        }
        else if constexpr (has_global_write<ValueType, Head, Tail...>()) {
            serialize(this->node, v, std::forward<Head>(head), std::forward<Tail>(tail)...);
        }
        else if constexpr (has_internal_write<ValueType, Head, Tail...>()) {
            this->node.write(v, std::forward<Head>(head), std::forward<Tail>(tail)...);
        }
        else {
            cant_serialize(v);
        }
    }

private:
    template <typename Type, typename ... Arguments>
    static constexpr bool has_internal_write() { return traits::template has_primitive_write<Type>::value || traits::template has_class_write<Type, Arguments...>::value; }
    template <typename Type, typename ... Arguments>
    static constexpr bool has_global_write() { return traits::template has_global_write<pack<Type, Arguments...>>::value; }

    struct traits {
        template <typename Type>
        using primitive_write_trait = std::void_t<
            std::enable_if_t<is_primitive_v<Type>>,
            decltype(static_cast<void(remove_cvref_t<Interface>::*)(remove_cvref_t<Type>)>(&remove_cvref_t<Interface>::write))
        >;
        template <typename, typename = void>
        struct has_primitive_write : std::false_type {};
        template <typename Type>
        struct has_primitive_write<Type, primitive_write_trait<Type>> : std::true_type {};

        template <typename Type, typename ... Arguments>
        using class_write_trait = std::void_t<
            std::enable_if_t<!is_primitive_v<Type>>,
            decltype(std::declval<Interface&>().write(std::declval<remove_cvref_t<Type>>(), std::declval<remove_cvref_t<Arguments>>()...))
        >;
        template <typename, typename = void, typename ...>
        struct has_class_write : std::false_type {};
        template <typename Type, typename ... Arguments>
        struct has_class_write<Type, class_write_trait<Type, Arguments...>, Arguments ...> : std::true_type {};

        template <typename Type, typename ... Arguments>
        using global_write_trait = std::void_t<
            std::enable_if_t<(!is_primitive_v<Type> || sizeof...(Arguments) > 0)>,
            decltype(serialize(std::declval<Interface&>(), std::declval<Type>(), std::declval<Arguments>()...))
        >;
        template <typename, typename = void>
        struct has_global_write : std::false_type {};
        template <typename Type, typename ... Arguments>
        struct has_global_write<pack<Type, Arguments...>, global_write_trait<remove_cvref_t<Type>, Arguments...>> : std::true_type {};
    };
};

template <typename Interface>
struct Object : Node<Interface> {
    using Base = Node<Interface>;

    Object(Interface& parent) : Base(parent) {
        this->node.beginObject();
    }

    ~Object() {
        this->node.endObject();
    }

    template <typename Key, typename Type = Object<Interface>>
    auto object(Key&& key) {
        this->node.writeKey(std::forward<Key>(key));
        return Type(this->node);
    }

    template <typename Key, typename Type = Array<Interface>>
    auto array(Key&& key) {
        this->node.writeKey(std::forward<Key>(key));
        return Type(this->node);
    }

    template <typename Key, typename Type = Blob<Interface>>
    auto blob(Key&& key) {
        this->node.writeKey(std::forward<Key>(key));
        return Type(this->node);
    }

    template <typename Key, typename ValueType>
    std::enable_if_t<is_primitive_v<ValueType>> value(Key&& key, ValueType v) {
        this->node.writeKey(std::forward<Key>(key));
        Base::value(v);
    }

    template <typename Key, typename ValueType, typename Functor>
    std::enable_if_t<is_primitive_v<ValueType>> value(Key&& key, ValueType v, Functor f) {
        this->node.writeKey(std::forward<Key>(key));
        Base::value(v, std::forward<Functor>(f));
    }

    template <typename Key, typename ValueType, typename ... Arguments>
    std::enable_if_t<(!is_primitive_v<ValueType> && !is_instantiation_of_v<std::optional, ValueType>)> value(Key&& key, const ValueType& v, Arguments&& ... args) {
        this->node.writeKey(std::forward<Key>(key));
        Base::value(v, std::forward<Arguments>(args)...);
    }

    template <typename Key, typename ValueType, typename ... Arguments>
    std::enable_if_t<is_instantiation_of_v<std::optional, ValueType>> value(Key&& key, const ValueType& v, Arguments&& ... args) {
        if (!v.has_value()) return;
        value(std::forward<Key>(key), v.value(), std::forward<Arguments>(args)...);
    }

    template <typename Key, typename ValueType, typename ... Arguments>
    std::enable_if_t<std::is_convertible_v<const ValueType, bool>> optional(Key&& key, const ValueType& v, Arguments&& ... args) {
        if (!v) return;
        value(std::forward<Key>(key), v, std::forward<Arguments>(args)...);
    }
};

template <typename Interface>
struct Array : Node<Interface> {
    using Base = Node<Interface>;
    using Base::value;

    Array(Interface& parent) : Base(parent) {
        this->node.beginArray();
    }

    ~Array() {
        this->node.endArray();
    }

    template <typename Type = Object<Interface>>
    auto object() { return Type(this->node); }
    template <typename Type = Array<Interface>>
    auto array() { return Type(this->node); }
    template <typename Type = Blob<Interface>>
    auto blob() { return Type(this->node); }
};

template <typename Interface>
struct Blob : Node<Interface> {
    using Base = Node<Interface>;
    using Base::value;

    Blob(Interface& parent) : Base(parent) {
        this->node.beginBlob();
    }

    ~Blob() {
        this->node.endBlob();
    }
};
}

namespace Clio {
template <typename Interface>
struct Serializer : Serialization::Node<Interface> {
    using Base = Serialization::Node<Interface>;
    using Base::value;

    Serializer() : Base(static_cast<Interface&>(*this)) {}
    Serializer(const Serializer&) = delete;
    Serializer& operator = (const Serializer&) = delete;

    template <typename Type = Serialization::Object<Interface>>
    auto object() { return Type(this->node); }
    template <typename Type = Serialization::Array<Interface>>
    auto array() { return Type(this->node); }
    template <typename Type = Serialization::Blob<Interface>>
    auto blob() { return Type(this->node); }
};
}
