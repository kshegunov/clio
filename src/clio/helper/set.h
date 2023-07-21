// SPDX-FileCopyrightText: © 2023 Konstantin Shegunov <kshegunov@gmail.com>
// SPDX-License-Identifier: MIT

#include "../Clio.h"
#include "common.h"
#include <set>

namespace Clio {
template <typename Interface, typename ... Args>
std::enable_if_t<is_serializer_v<Interface>> serialize(Interface& s, const std::set<Args...>& v) {
    detail::serialize_sequence(s, v);
}

template <typename Interface, typename ... Args>
std::enable_if_t<is_deserializer_v<Interface>> deserialize(Interface& d, std::set<Args...>& v) {
    detail::deserialize_sequence(d, v);
}
}
