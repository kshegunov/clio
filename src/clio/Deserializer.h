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
    void readValue(ValueType& v) {
        if constexpr (traits::template has_global_read<ValueType>::value) {
            deserialize(this->node, v);
        }
        else if constexpr (traits::template has_internal_read<ValueType>::value) {
            this->node.read(v);
        }
        else {
            cant_deserialize(v);
        }
    }

private:
    struct traits {
        template <typename Type>
        using internal_read_trait = std::void_t<
            decltype(static_cast<void(remove_cvref_t<Interface>::*)(Type&)>(&remove_cvref_t<Interface>::read))
        >;
        template <typename, typename = void>
        struct has_internal_read : std::false_type {};
        template <typename Type>
        struct has_internal_read<Type, internal_read_trait<Type>> : std::true_type {};

        template <typename Type>
        using global_read_trait = std::void_t<
            decltype(deserialize(std::declval<Interface&>(), std::declval<Type&>())),
            std::enable_if_t<!is_primitive_v<Type>>
        >;
        template <typename, typename = void>
        struct has_global_read : std::false_type {};
        template <typename Type>
        struct has_global_read<Type, global_read_trait<Type>> : std::true_type {};
    };
};

template <typename Interface>
struct Object : Node<Interface> {
    Object(Interface& parent) : Node<Interface>(parent) {
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

    template <typename Key>
    auto object(Key&& key) {
        this->node.readKey(std::forward<Key>(key));
        return Object(this->node);
    }

    template <typename Key>
    auto array(Key&& key) {
        this->node.readKey(std::forward<Key>(key));
        return Array(this->node);
    }

    template <typename Key>
    auto blob(Key&& key) {
        this->node.readKey(std::forward<Key>(key));
        return Blob(this->node);
    }

    template <typename Key, typename ValueType>
    void value(Key&& key, ValueType& v) {
        this->node.readKey(std::forward<Key>(key));
        this->readValue(v);
    }

    template <typename Key, typename ValueType>
    void value(Key&& key, const std::optional<ValueType>& v) {
        if (!hasKey(key)) return;
        value(std::forward<Key>(key), v.value());
    }

    auto empty() const { return !size(); }
    auto size() const { return this->node.size(); }
};

template <typename Interface>
struct Array : Node<Interface> {
    Array(Interface& parent) : Node<Interface>(parent) {
        this->node.beginArray();
    }
    ~Array() {
        this->node.endArray();
    }

    auto object() { return Object(this->node); }
    auto array() { return Array(this->node); }
    auto blob() { return Blob(this->node); }

    template <typename ValueType>
    void value(ValueType& v) {
        this->readValue(v);
    }

    auto empty() const { return !size(); }
    auto size() const { return this->node.size(); }
};

template <typename Interface>
struct Blob : Node<Interface> {
    Blob(Interface& parent) : Node<Interface>(parent) {
        this->node.beginBlob();
    }
    ~Blob() {
        this->node.endBlob();
    }

    template <typename ValueType>
    void value(ValueType& v) {
        this->readValue(v);
    }
};
}

namespace Clio {
template <typename Interface>
struct Deserializer : Deserialization::Node<Interface> {
    Deserializer() : Deserialization::Node<Interface>(static_cast<Interface&>(*this)) {}

    auto object() { return Deserialization::Object(this->node); }
    auto array() { return Deserialization::Array(this->node); }
    auto blob() { return Deserialization::Blob(this->node); }

    template <typename ValueType>
    void value(ValueType& v) {
        this->readValue(v);
    }
};
}
