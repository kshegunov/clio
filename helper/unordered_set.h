#include "../Clio.h"
#include "common.h"
#include <unordered_set>

namespace Clio {
template <typename Interface, typename ... Args>
std::enable_if_t<is_serializer_v<Interface>> serialize(Interface& s, const std::unordered_set<Args...>& v) {
    detail::serialize_sequence(s, v);
}

template <typename Interface, typename ... Args>
std::enable_if_t<is_deserializer_v<Interface>> deserialize(Interface& d, std::unordered_set<Args...>& v) {
    detail::deserialize_sequence(d, v);
}
}
