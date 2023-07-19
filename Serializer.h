#pragma once

#include <type_traits>
#include <utility>
#include <string_view>
#include <optional>

namespace Clio {
template <typename, typename>
struct Serializer;
template <typename>
struct Deserializer;
template <typename>
struct Object;
template <typename>
struct Array;

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename = void, typename ... Args>
struct require_all : std::false_type {};
template <typename ... Args>
struct require_all<std::void_t<Args...>, Args...> : std::true_type {};
template <typename ... Args>
inline constexpr bool require_all_v = require_all<Args...>::value;

template <typename Interface, typename = void>
struct serializer_type {
    using type = void;
};
template <typename Interface>
struct serializer_type<Interface, std::void_t<typename Interface::Backend>> {
    using type = Serializer<Interface, typename Interface::Backend>;
};
template <typename Interface>
inline constexpr bool is_serializer_v = std::is_base_of_v<typename serializer_type<Interface>::type, Interface>;
template <typename Implementation>
inline constexpr bool is_deserializer_v =  std::is_base_of_v<Deserializer<Implementation>, Implementation>;

template <typename Type>
inline constexpr bool is_primitive_v = std::is_arithmetic_v<remove_cvref_t<Type>> || std::is_pointer_v<remove_cvref_t<Type>>;

namespace Serialization {
template <typename Type>
using write_argument_t = std::conditional_t<is_primitive_v<Type>, remove_cvref_t<Type>, std::add_lvalue_reference_t<std::add_const_t<Type>>>;
template <typename Implementation, typename Type>
using internal_write_prototype_t = void(remove_cvref_t<Implementation>::*)(write_argument_t<Type>);
template <typename Implementation, typename Type>
using internal_write_trait = std::void_t<decltype(static_cast<internal_write_prototype_t<Implementation, Type>>(&remove_cvref_t<Implementation>::write))>;
template <typename, typename, typename = void>
struct has_internal_write : std::false_type {};
template <typename Implementation, typename Type>
struct has_internal_write<Implementation, Type, internal_write_trait<Implementation, Type>> : std::true_type {};

template <typename Implementation, typename Type>
using own_write_trait = std::void_t<decltype(std::declval<Type&>().serialize(std::declval<Implementation&>()))>;
template <typename, typename, typename = void>
struct has_own_write : std::false_type {};
template <typename Implementation, typename Type>
struct has_own_write<Implementation, Type, own_write_trait<Implementation, Type>> : std::true_type {};

template <typename Implementation, typename Type>
using global_write_trait = std::void_t<
    decltype(serialize(std::declval<Implementation&>(), std::declval<const Type&>())),
    std::enable_if_t<!std::is_same_v<remove_cvref_t<Type>, write_argument_t<Type>>>
>;
template <typename, typename, typename = void>
struct has_global_write : std::false_type {};
template <typename Implementation, typename Type>
struct has_global_write<Implementation, Type, global_write_trait<Implementation, Type>> : std::true_type {};

template <typename Interface>
inline constexpr bool is_tainted = sizeof(Interface) != sizeof(Serializer<Interface, typename Interface::Backend>);
template <typename Self, typename Other>
inline constexpr bool can_convert_v = is_serializer_v<Other> && std::is_base_of_v<typename Other::Backend, typename Self::Backend> && !std::is_same_v<Self, Other> && !is_tainted<Self> && !is_tainted<Other>;

template <typename Interface>
struct Node {
protected:
    constexpr Node(Interface& parent) : node(parent) {}
    Interface& node;
};
}

template <typename Interface, typename Implementation>
struct Serializer : protected Implementation {
    using Self = Interface;
    using Backend = Implementation;

    struct Object;
    struct Array;

    template <typename ... Args>
    Serializer(Args&& ... args) : Implementation(std::forward<Args>(args)...) {}

    template <typename Other, typename = std::enable_if_t<Serialization::can_convert_v<Self, Other>>>
    operator Other& ()& {
        // Facilitates overloading serializers with the base; safe upcast:
        // The overload is disabled if the interface is tainted or if the backends are unrelated
        return reinterpret_cast<Other&>(*this);
    }

    auto object() { return Object(i()); }
    auto array() { return Array(i()); }

    template <typename ValueType>
    std::enable_if_t<is_primitive_v<ValueType>> value(const ValueType v) {
        using namespace Serialization;
        constexpr bool has_internal_write_v = has_internal_write<Backend, ValueType>::value;
        static_assert(has_internal_write_v, "Implementation doesn't support this type");
        i().write(v);
    }
    template <typename ValueType>
    std::enable_if_t<!is_primitive_v<ValueType>> value(const ValueType& v) {
        using namespace Serialization;
        constexpr bool has_own_write_v = has_own_write<Self, const ValueType&>::value;
        constexpr bool has_global_write_v = has_global_write<Self, const ValueType&>::value;
        constexpr bool has_internal_write_v = has_internal_write<Backend, const ValueType&>::value;
        static_assert((has_internal_write_v || has_global_write_v || has_own_write_v), "Implementation doesn't support this type & there's no overload to handle the serialization");
        if constexpr (has_global_write_v) {
            serialize(i(), v);
        }
        else if constexpr (has_own_write_v) {
            v.serialize(i());
        }
        else if constexpr (has_internal_write_v){
            i().write(v);
        }
    }

protected:
    constexpr Self& i() noexcept { return *static_cast<Self*>(this); }
    constexpr const Self& i() const noexcept { return *static_cast<const Self*>(this); }
};

template <typename Interface, typename Implementation>
struct Serializer<Interface, Implementation>::Object : Serialization::Node<Interface> {
    Object(Interface& parent) : Serialization::Node<Interface>(parent) {
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
        this->node.value(v);
    }
    template <typename Key, typename ValueType>
    std::enable_if_t<!is_primitive_v<ValueType>> value(Key&& key, const ValueType& v) {
        this->node.pushKey(std::forward<Key>(key));
        this->node.value(v);
    }
    template <typename Key, typename ValueType>
    void value(Key&& key, const std::optional<ValueType>& v) {
        if (!v) return;
        value(std::forward<Key>(key), v.value());
    }
};

template <typename Interface, typename Implementation>
struct Serializer<Interface, Implementation>::Array : Serialization::Node<Interface> {
    Array(Interface& parent) : Serialization::Node<Interface>(parent) {
        this->node.pushArray();
    }
    ~Array() {
        this->node.pushArray();
    }

    auto object() { return Object(this->node); }
    auto array() { return Array(this->node); }

    template <typename ValueType>
    std::enable_if_t<is_primitive_v<ValueType>> value(const ValueType v) {
        this->node.value(v);
    }

    template <typename ValueType>
    std::enable_if_t<!is_primitive_v<ValueType>> value(const ValueType& v) {
        this->node.value(v);
    }

    template <typename ValueType>
    void value(const std::optional<ValueType>& v) {
        if (!v) return;
        value(v.value());
    }
};
template <typename Interface>
Array(Interface&) -> Array<Interface>;
}
