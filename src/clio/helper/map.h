// SPDX-FileCopyrightText: © 2023 Konstantin Shegunov <kshegunov@gmail.com>
// SPDX-License-Identifier: MIT

#include "../Clio.h"
#include "common.h"
#include <map>

namespace Clio {
template <typename Interface, typename ... Args>
std::enable_if_t<is_serializer_v<Interface>> serialize(Interface& s, const std::map<Args...>& v) {
    detail::serialize_associative(s, v);
}

template <typename Interface, typename ... Args>
std::enable_if_t<is_deserializer_v<Interface>> deserialize(Interface& d, std::map<Args...>& v) {
    detail::deserialize_associative(d, v);
}

template <typename Interface, typename Functor, typename ... Args>
std::enable_if_t<is_serializer_v<Interface>> serialize(Interface& s, const std::map<Args...>& v, Functor f) {
    detail::serialize_associative(s, v, std::forward<Functor>(f));
}

template <typename Interface, typename Functor, typename ... Args>
std::enable_if_t<Clio::is_deserializer_v<Interface>> deserialize(Interface& d, std::map<Args...>& v, Functor f) {
    detail::deserialize_associative(d, v, std::forward<Functor>(f));
}
}
