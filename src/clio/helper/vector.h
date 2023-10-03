// SPDX-FileCopyrightText: © 2023 Konstantin Shegunov <kshegunov@gmail.com>
// SPDX-License-Identifier: MIT

#include "../Clio.h"
#include "common.h"
#include <vector>

namespace Clio {
template <typename Interface, typename ... Parameters, typename ... Arguments>
std::enable_if_t<is_serializer_v<Interface>> serialize(Interface& s, const std::vector<Parameters...>& v, Arguments&& ... args) {
    detail::serialize_sequence(s, v, std::forward<Arguments>(args)...);
}

template <typename Interface, typename ... Parameters, typename ... Arguments>
std::enable_if_t<is_deserializer_v<Interface>> deserialize(Interface& d, std::vector<Parameters...>& v, Arguments&& ... args) {
    detail::deserialize_sequence(d, v, std::forward<Arguments>(args)...);
}
}
