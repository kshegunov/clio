// SPDX-FileCopyrightText: © 2023 Konstantin Shegunov <kshegunov@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once
#include "Clio.h"
#include <utility>
#include <optional>

namespace Clio::Deserialization {
template <typename Interface>
struct Node : Clio::Node<Interface> {
    using Base = Clio::Node<Interface>;
    using Base::Base;

protected:
    template <typename ValueType>
    void value(ValueType& v) {
        if constexpr (has_global_read<ValueType>()) {
            deserialize(this->node, v);
        }
        else if constexpr (has_internal_read<ValueType>()) {
            this->node.read(v);
        }
        else {
            cant_deserialize(v);
        }
    }

    template <typename ValueType, typename Head, typename ... Tail>
    void value(ValueType& v, Head&& head, Tail&& ... tail) {
        if constexpr (std::is_invocable_v<Head, Interface&, ValueType&, Tail...>) {
            std::forward<Head>(head)(this->node, v, std::forward<Tail>(tail)...);
        }
        else if constexpr (has_global_read<ValueType, Head, Tail...>()) {
            deserialize(this->node, v, std::forward<Head>(head), std::forward<Tail>(tail)...);
        }
        else if constexpr (has_internal_read<ValueType, Head, Tail...>()) {
            this->node.read(v, std::forward<Head>(head), std::forward<Tail>(tail)...);
        }
        else {
            cant_deserialize(v);
        }
    }

private:
    template <typename Type, typename ... Arguments>
    static constexpr bool has_internal_read() { return traits::template has_internal_read<pack<Type, Arguments...>>::value; }
    template <typename Type, typename ... Arguments>
    static constexpr bool has_global_read() { return traits::template has_global_read<pack<Type, Arguments...>>::value; }

    struct traits {
        template <typename Type, typename ... Arguments>
        using internal_read_trait = std::void_t<
            decltype(std::declval<Interface&>().read(std::declval<Type&>(), std::declval<remove_cvref_t<Arguments>>()...))
        >;
        template <typename, typename = void>
        struct has_internal_read : std::false_type {};
        template <typename Type, typename ... Arguments>
        struct has_internal_read<pack<Type, Arguments...>, internal_read_trait<Type, Arguments...>> : std::true_type {};

        template <typename Type, typename ... Arguments>
        using global_read_trait = std::void_t<
            std::enable_if_t<(!is_primitive_v<Type> || sizeof...(Arguments) > 0)>,
            decltype(deserialize(std::declval<Interface&>(), std::declval<Type&>(), std::declval<Arguments>()...))
        >;
        template <typename, typename = void>
        struct has_global_read : std::false_type {};
        template <typename Type, typename ... Arguments>
        struct has_global_read<pack<Type, Arguments...>, global_read_trait<remove_cvref_t<Type>, Arguments...>> : std::true_type {};
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

    auto peekKey() const {
        return this->node.peekKey();
    }

    template <typename Key>
    bool hasKey(const Key& key) {
        return this->node.hasKey(key);
    }

    template <typename Key, typename Type = Object<Interface>>
    auto object(Key&& key) {
        this->node.readKey(std::forward<Key>(key));
        return Type(this->node);
    }

    template <typename Key, typename Type = Array<Interface>>
    auto array(Key&& key) {
        this->node.readKey(std::forward<Key>(key));
        return Type(this->node);
    }

    template <typename Key, typename Type = Blob<Interface>>
    auto blob(Key&& key) {
        this->node.readKey(std::forward<Key>(key));
        return Type(this->node);
    }

    template <typename Key, typename ValueType, typename ... Arguments>
    std::enable_if_t<(!is_instantiation_of_v<std::optional, ValueType>)> value(Key&& key, ValueType& v, Arguments&& ... args) {
        this->node.readKey(std::forward<Key>(key));
        Base::value(v, std::forward<Arguments>(args)...);
    }

    template <typename Key, typename ValueType, typename ... Arguments>
    std::enable_if_t<is_instantiation_of_v<std::optional, ValueType>> value(Key&& key, ValueType& v, Arguments&& ... args) {
        if (!hasKey(key)) return;
        typename ValueType::value_type result;
        value(std::forward<Key>(key), result, std::forward<Arguments>(args)...);
        v = std::move(result);
    }

    template <typename Key, typename ValueType, typename ... Arguments>
    void optional(Key&& key, ValueType& v, Arguments&& ... args) {
        if (!hasKey(key)) return;
        value(std::forward<Key>(key), v, std::forward<Arguments>(args)...);
    }

    auto empty() const { return !size(); }
    auto size() const { return this->node.size(); }
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

    auto empty() const { return !size(); }
    auto size() const { return this->node.size(); }
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
struct Deserializer : Deserialization::Node<Interface> {
    using Base = Deserialization::Node<Interface>;
    using Base::value;

    Deserializer() : Base(static_cast<Interface&>(*this)) {}

    template <typename Type = Deserialization::Object<Interface>>
    auto object() { return Type(this->node); }
    template <typename Type = Deserialization::Array<Interface>>
    auto array() { return Type(this->node); }
    template <typename Type = Deserialization::Blob<Interface>>
    auto blob() { return Type(this->node); }

    template <typename ValueType, typename ... Arguments>
    ValueType root(Arguments&& ... args) {
        ValueType v;
        value(v, std::forward<Arguments>(args)...);
        return v;
    }
};
}
