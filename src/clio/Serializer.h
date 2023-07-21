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
    void writeValue(ValueType v) {
        if constexpr (traits::template has_global_write<ValueType>::value) {
            serialize(this->node, std::forward<ValueType>(v));
        }
        else if constexpr (traits::template has_internal_write<ValueType>::value) {
            this->node.write(std::forward<ValueType>(v));
        }
        else {
            cant_serialize(v);
        }
    }

private:
    struct traits {
        template <typename Type, typename = void>
        struct has_internal_primitive_write : std::false_type {};
        template <typename Type>
        struct has_internal_primitive_write<Type,
            std::void_t<
                std::enable_if_t<is_primitive_v<Type>>,
                decltype(static_cast<void(remove_cvref_t<Interface>::*)(remove_cvref_t<Type>)>(&remove_cvref_t<Interface>::write))
            >
        > : std::true_type {};
        template <typename Type, typename = void>
        struct has_internal_class_write : std::false_type {};
        template <typename Type>
        struct has_internal_class_write<Type,
            std::void_t<
                std::enable_if_t<!is_primitive_v<Type>>,
                decltype(std::declval<Interface&>().write(std::declval<Type>()))
            >
        > : std::true_type {};
        template <typename Type>
        using has_internal_write = std::bool_constant<has_internal_primitive_write<Type>::value || has_internal_class_write<Type>::value>;

        template <typename Type>
        using global_write_trait = std::void_t<
            decltype(serialize(std::declval<Interface&>(), std::declval<const Type&>())),
            std::enable_if_t<!is_primitive_v<Type>>
        >;
        template <typename, typename = void>
        struct has_global_write : std::false_type {};
        template <typename Type>
        struct has_global_write<Type, global_write_trait<Type>> : std::true_type {};
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

    template <typename Key>
    auto object(Key&& key) {
        this->node.writeKey(std::forward<Key>(key));
        return Object(this->node);
    }

    template <typename Key>
    auto array(Key&& key) {
        this->node.writeKey(std::forward<Key>(key));
        return Array(this->node);
    }

    template <typename Key>
    auto blob(Key&& key) {
        this->node.writeKey(std::forward<Key>(key));
        return Blob(this->node);
    }

    template <typename Key, typename ValueType>
    std::enable_if_t<is_primitive_v<ValueType>> value(Key&& key, const ValueType v) {
        this->node.writeKey(std::forward<Key>(key));
        this->writeValue(v);
    }
    template <typename Key, typename ValueType>
    std::enable_if_t<!is_primitive_v<ValueType>> value(Key&& key, const ValueType& v) {
        this->node.writeKey(std::forward<Key>(key));
        this->writeValue(v);
    }

    template <typename Key, typename ValueType>
    void value(Key&& key, const std::optional<ValueType>& v) {
        if (!v.has_value()) return;
        value(std::forward<Key>(key), v.value());
    }
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
    std::enable_if_t<is_primitive_v<ValueType>> value(const ValueType v) {
        this->writeValue(v);
    }

    template <typename ValueType>
    std::enable_if_t<!is_primitive_v<ValueType>> value(const ValueType& v) {
        this->writeValue(v);
    }

    template <typename ValueType>
    void value(const std::optional<ValueType>& v) {
        if (!v.has_value()) return;
        value(v.value());
    }
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
    std::enable_if_t<is_primitive_v<ValueType>> value(const ValueType v) {
        this->writeValue(v);
    }

    template <typename ValueType>
    std::enable_if_t<!is_primitive_v<ValueType>> value(const ValueType& v) {
        this->writeValue(v);
    }
};
}

namespace Clio {
template <typename Interface>
struct Serializer : Serialization::Node<Interface> {
    Serializer() : Serialization::Node<Interface>(static_cast<Interface&>(*this)) {}
    Serializer(const Serializer&) = delete;
    Serializer& operator = (const Serializer&) = delete;

    auto object() { return Serialization::Object(this->node); }
    auto array() { return Serialization::Array(this->node); }
    auto blob() { return Serialization::Blob(this->node); }

    template <typename ValueType>
    std::enable_if_t<Clio::is_primitive_v<ValueType>> value(const ValueType v) {
        this->writeValue(v);
    }
    template <typename ValueType>
    std::enable_if_t<!Clio::is_primitive_v<ValueType>> value(const ValueType& v) {
        this->writeValue(v);
    }
};
}
