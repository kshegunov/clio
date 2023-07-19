#pragma once

#include <type_traits>
#include <utility>
#include <string_view>
#include <optional>

namespace Clio {
template <typename>
struct Serializer;
template <typename>
struct Deserializer;
template <typename>
struct Object;
template <typename>
struct Array;

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename... Args>
struct require_all {
    using type = void;
};
template <typename... Args>
using require_all_t = typename require_all<Args...>::type;
template <typename Implementation>
inline constexpr bool is_serializer_v = std::is_base_of_v<Serializer<Implementation>, Implementation>;
template <typename Implementation>
inline constexpr bool is_deserializer_v =  std::is_base_of_v<Deserializer<Implementation>, Implementation>;

template <typename Type>
inline constexpr bool is_primitive_v = std::is_arithmetic_v<remove_cvref_t<Type>> || std::is_pointer_v<remove_cvref_t<Type>>;

namespace Serialization {
template <typename Type>
using write_argument_t = std::conditional_t<is_primitive_v<Type>, remove_cvref_t<Type>, std::add_lvalue_reference_t<std::add_const_t<Type>>>;
template <typename Implementation, typename Type>
using write_prototype_t = void(remove_cvref_t<Implementation>::*)(write_argument_t<Type>);
template <typename, typename, typename = void>
struct write_implementation {
    using type = void;
};
template <typename Implementation, typename Type>
struct write_implementation<Implementation, Type, std::void_t<decltype(static_cast<write_prototype_t<Implementation, Type>>(&remove_cvref_t<Implementation>::write))>> {
    using type = write_prototype_t<Implementation, Type>;
};
template <typename Implementation, typename Type>
using write_implementation_t = typename write_implementation<Implementation, Type>::type;

template <typename Implementation, typename Type>
using own_write_trait = require_all_t<
    decltype(std::declval<Type&>().serialize(std::declval<Implementation&>()))
>;
template <typename, typename, typename = void>
struct has_own_write : std::false_type {};
template <typename Implementation, typename Type>
struct has_own_write<Implementation, Type, own_write_trait<Implementation, Type>> : std::true_type {};

template <typename Implementation, typename Type>
using global_write_trait = require_all_t<
    std::void_t<decltype(serialize(std::declval<Implementation&>(), std::declval<const Type&>()))>,
    std::enable_if_t<!std::is_same_v<remove_cvref_t<Type>, write_argument_t<Type>>>
>;
template <typename, typename, typename = void>
struct has_global_write : std::false_type {};
template <typename Implementation, typename Type>
struct has_global_write<Implementation, Type, global_write_trait<Implementation, Type>> : std::true_type {};




template <typename Implementation>
using object_trait_t = require_all_t<decltype(&Implementation::pushObject), decltype(&Implementation::popObject)>;
template <typename, typename = void>
struct resolve_object {
    using type = void;
};
template <typename Implementation>
struct resolve_object<Implementation, object_trait_t<Implementation>> {
    using type = Object<Implementation>;
};
template <typename Implementation>
using resolve_object_t = typename resolve_object<Implementation>::type;

template <typename Implementation>
using array_trait_t = require_all_t<decltype(&Implementation::pushArray), decltype(&Implementation::popArray)>;
template <typename Implementation, typename = void>
struct resolve_array {
    using type = void;
};
template <typename Implementation>
struct resolve_array<Implementation, array_trait_t<Implementation>> {
    using type = Array<Implementation>;
};
template <typename Implementation>
using resolve_array_t = typename resolve_array<Implementation>::type;

template <typename Interface>
struct Pushed {
    using Node = typename Interface::Self;

protected:
    constexpr Pushed(Node& parent) : node(parent) {}
    Node& node;
};

template <typename Interface>
struct Array : Pushed<Interface> {
    using Base = Pushed<Interface>;
    static_assert(!std::is_void_v<resolve_array_t<Interface>>, "Implementation doesn't support an array interface");

    Array(Interface& parent) : Base(parent) {
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

template <typename Interface>
struct Object : Pushed<Interface> {
    using Base = Pushed<Interface>;
    static_assert(!std::is_void_v<resolve_object_t<Interface>>, "Implementation doesn't support an object interface");

    Object(Interface& parent) : Base(parent) {
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
template <typename Interface>
Object(Interface&) -> Object<Interface>;
}

template <typename Implementation>
struct Serializer {
    using Self = Implementation;

    template <typename Type>
    inline static constexpr bool has_own_write_v = Serialization::has_own_write<Implementation, Type>::value;
    template <typename Type>
    inline static constexpr bool has_global_write_v = Serialization::has_global_write<Implementation, Type>::value;
    template <typename Type>
    inline static constexpr bool has_write_implementation_v = !std::is_void_v<Serialization::write_implementation_t<Implementation, Type>>;
    template <typename Type>
    inline static constexpr bool has_write_v = has_write_implementation_v<Type> || (!is_primitive_v<Type> && (has_global_write_v<Type> || has_own_write_v<Type>));

    auto object() { return Serialization::Object(i()); }
    auto array() { return Serialization::Array(i()); }

    template <typename ValueType>
    std::enable_if_t<is_primitive_v<ValueType>> value(const ValueType v) {
        using namespace Serialization;
        static_assert(has_write_v<ValueType>, "Implementation doesn't support this type");
        i().write(v);
    }
    template <typename ValueType>
    std::enable_if_t<!is_primitive_v<ValueType>> value(const ValueType& v) {
        using namespace Serialization;
        static_assert(has_write_v<ValueType>, "Implementation doesn't support this type & there's no overload to handle the serialization");
        if constexpr (has_global_write_v<ValueType>) {
            serialize(i(), v);
        }
        else if constexpr (has_own_write_v<ValueType>) {
            v.serialize(i());
        }
        else if constexpr (has_write_implementation_v<ValueType>){
            i().write(v);
        }
    }

protected:
    constexpr Self& i() noexcept { return *static_cast<Self*>(this); }
    constexpr const Self& i() const noexcept { return *static_cast<const Self*>(this); }
};
}
