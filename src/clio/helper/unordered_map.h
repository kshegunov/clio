// SPDX-FileCopyrightText: © 2023 Konstantin Shegunov <kshegunov@gmail.com>
// SPDX-License-Identifier: MIT

#include "../Clio.h"
#include "common.h"
#include <unordered_map>

namespace Clio {
template <typename Interface, typename ... Parameters, typename ... Arguments>
std::enable_if_t<is_serializer_v<Interface>> serialize(Interface& s, const std::unordered_map<Parameters...>& v, Arguments&& ... args) {
    detail::serialize_associative(s, v, std::forward<Arguments>(args)...);
}

template <typename Interface, typename ... Parameters, typename ... Arguments>
std::enable_if_t<is_deserializer_v<Interface>> deserialize(Interface& d, std::unordered_map<Parameters...>& v, Arguments&& ... args) {
    detail::deserialize_associative(d, v, std::forward<Arguments>(args)...);
}
}
