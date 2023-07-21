// SPDX-FileCopyrightText: © 2023 Konstantin Shegunov <kshegunov@gmail.com>
// SPDX-License-Identifier: MIT

#include "../Clio.h"
#include "common.h"
#include <unordered_map>

namespace Clio {
template <typename Interface, typename ... Args>
std::enable_if_t<is_serializer_v<Interface>> serialize(Interface& s, const std::unordered_map<Args...>& v) {
    detail::serialize_associative(s, v);
}

template <typename Interface, typename ... Args>
std::enable_if_t<is_deserializer_v<Interface>> deserialize(Interface& d, std::unordered_map<Args...>& v) {
    detail::deserialize_associative(d, v);
}
}
