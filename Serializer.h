#pragma once
#include "Clio.h"
#include <optional>

namespace Clio::Serialization {
template <typename Interface>
struct Array;
template <typename Interface>
struct Object;
template <typename Interface>
struct Blob;

template <typename Interface>
struct Node : Clio::Node<Interface> {
    using Base = Clio::Node<Interface>;
    using Base::Base;

protected:
    template <typename ValueType>
    void pushValue(ValueType v) {
        if constexpr (traits::template has_global_write<ValueType>::value) {
            serialize(this->node, std::forward<ValueType>(v));
        }
        else if constexpr (traits::template has_internal_write<ValueType>::value){
            this->node.write(std::forward<ValueType>(v));
        }
        else {
            cant_serialize(v);
        }
    }

private:
    struct traits {
        template <typename Type>
        using write_argument_t = std::conditional_t<is_primitive_v<Type>, remove_cvref_t<Type>, std::add_lvalue_reference_t<std::add_const_t<Type>>>;

        template <typename Type>
        using internal_write_trait = std::void_t<
            decltype(static_cast<void(remove_cvref_t<Interface>::*)(write_argument_t<Type>)>(&remove_cvref_t<Interface>::write))
        >;
        template <typename, typename = void>
        struct has_internal_write : std::false_type {};
        template <typename Type>
        struct has_internal_write<Type, internal_write_trait<Type>> : std::true_type {};

        template <typename Type>
        using global_write_trait = std::void_t<
            decltype(serialize(std::declval<Interface&>(), std::declval<const Type&>())),
            std::enable_if_t<!std::is_same_v<remove_cvref_t<Type>, write_argument_t<Type>>>
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

    template <typename Key>
    auto blob(Key&& key) {
        this->node.pushKey(std::forward<Key>(key));
        return Blob(this->node);
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
        this->node.popArray();
    }

    auto object() { return Object(this->node); }
    auto array() { return Array(this->node); }
    auto blob() { return Blob(this->node); }

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

template <typename Interface>
struct Blob : Node<Interface> {
    Blob(Interface& parent) : Node<Interface>(parent) {
        this->node.pushBlob();
    }
    ~Blob() {
        this->node.popBlob();
    }

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

namespace Clio {
template <typename Interface>
struct Serializer : Serialization::Node<Interface> {
    Serializer() : Serialization::Node<Interface>(static_cast<Interface&>(*this)) {}

    auto object() { return Serialization::Object(this->node); }
    auto array() { return Serialization::Array(this->node); }
    auto blob() { return Serialization::Blob(this->node); }

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
