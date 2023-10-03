// SPDX-FileCopyrightText: © 2023 Konstantin Shegunov <kshegunov@gmail.com>
// SPDX-License-Identifier: MIT

#include "../Clio.h"
#include "common.h"
#include <array>

namespace Clio {
template <typename Interface, typename Type, std::size_t Size, typename ... Arguments>
std::enable_if_t<is_serializer_v<Interface>> serialize(Interface& s, const Type (&v)[Size], Arguments&& ... args) {
    detail::serialize_fixed_sequence(s, v, std::forward<Arguments>(args)...);
}

template <typename Interface, typename Type, std::size_t Size, typename ... Arguments>
std::enable_if_t<is_deserializer_v<Interface>> deserialize(Interface& d, Type (&v)[Size], Arguments&& ... args) {
    detail::deserialize_fixed_sequence(d, v, std::forward<Arguments>(args)...);
}

template <typename Interface, typename Type, std::size_t Size, typename ... Arguments>
std::enable_if_t<is_serializer_v<Interface>> serialize(Interface& s, const std::array<Type, Size>& v, Arguments&& ... args) {
    detail::serialize_fixed_sequence(s, v, std::forward<Arguments>(args)...);
}

template <typename Interface, typename Type, std::size_t Size, typename ... Arguments>
std::enable_if_t<Clio::is_deserializer_v<Interface>> deserialize(Interface& d, std::array<Type, Size>& v, Arguments&& ... args) {
    detail::deserialize_fixed_sequence(d, v, std::forward<Arguments>(args)...);
}
}
