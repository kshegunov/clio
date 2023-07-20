#include "../Clio.h"
#include "common.h"
#include <vector>

namespace Clio {
template <typename Interface, typename ... Args>
std::enable_if_t<is_serializer_v<Interface>> serialize(Interface& s, const std::vector<Args...>& v) {
    detail::serialize_sequence(s, v);
}

template <typename Interface, typename ... Args>
std::enable_if_t<is_deserializer_v<Interface>> deserialize(Interface& d, std::vector<Args...>& v) {
    detail::deserialize_sequence(d, v);
}
}
