#include <iostream>
#include "Serializer.h"
#include <cmath>


struct JsonBackend {
    JsonBackend(int = 0) {}
    void pushArray() {}
    void popArray() {}

    void pushObject() {}
    void popObject() {}

    void pushKey(std::string_view) {}
    void write(int) {
        int q = 1;
    }
};
struct JsonBackendExt : JsonBackend {
    using JsonBackend::write;
    void write(double) {}
};

struct JsonSerializer final : Clio::Serializer<JsonSerializer>, protected JsonBackend {
    friend struct Clio::Serializer<JsonSerializer>;
    friend struct Clio::Serialization::Object<JsonSerializer>;
    friend struct Clio::Serialization::Array<JsonSerializer>;
    template <typename, typename> friend struct Clio::Serialization::resolve_object;
    template <typename, typename> friend struct Clio::Serialization::resolve_array;
    template <typename, typename, typename> friend struct Clio::Serialization::write_implementation;
    template <typename ... Args>
    JsonSerializer(Args&& ... args) : JsonBackend(std::forward<Args>(args)...) {}
};

struct JsonSecondary : Clio::Serializer<JsonSecondary>, JsonBackendExt {
};

struct Bla {
    int x;
};

template <typename Implementation>
void serialize(Clio::Serializer<Implementation>& s, const Bla& arg) {
    s.value(arg.x);
}

template <typename WeakType>
std::enable_if_t<std::is_same_v<WeakType, Bla>> serialize(JsonSerializer& s, const WeakType& arg) {
    s.value(arg.x);
}

//template <typename Serializer>
//void serialize(Serializer& s, double z) {
//    int q = std::round(z);
//    s.value(q);
//}

//void serialize(JsonSerializer& s, const Bla& m) {
////    int q = std::round(z);
//    s.value(m.x);
//}

int main()
{
    JsonSecondary s2;
    JsonSerializer s;
    auto arr = s.object();
    auto z = arr.object("asdsaA");
    auto q = z.array("asfkjsaf");
    int x;
    std::optional<int> mqwe = 1;
    q.value(x);
    z.value("adasra", x);
    z.value("z", mqwe);
    s.value(x);
    double y;

    Bla m;
    z.value("Asd", m);

    auto ga = s2.object();
    ga.value("asdas", m);
    s2.value(y);

    return 0;
}
