#include <iostream>
#include "Serializer.h"
#include <cmath>

#include <vector>
struct JsonBackend {
    JsonBackend(int = 0) {}
    void pushObject() {}
    void popObject() {}

    void pushKey(std::string_view) {}
    void write(int) {
        int q = 1;
    }

    std::vector<int> x = { 1, 2, 3 };
};
struct JsonBackendExt : JsonBackend {
    void pushArray() {}
    void popArray() {}

    using JsonBackend::write;
    void write(double) {}
    std::vector<double> y = { 4, 5, 6 };
};

struct UBackend {
};

struct JsonSerializer final : Clio::Serializer<JsonSerializer, JsonBackend> {
    using Base = Clio::Serializer<JsonSerializer, JsonBackend>;
    using Base::Base;
};

struct JsonSecondary final : Clio::Serializer<JsonSecondary, JsonBackendExt> {
//    template <typename Type, typename = std::enable_if_t<Clio::is_serializer_v<Type>>>
//    operator Type& ()& {
//        return *this;
//    }
};

struct USerialize : Clio::Serializer<USerialize, UBackend> {
};

struct Bla {
    int x;
};

void serialize(JsonSerializer& s, const Bla& arg) {
    s.value(arg.x);
}

//void serialize(JsonSecondary& s, const Bla& arg) {
//    s.value(arg.x);
//}

void serialize(USerialize& s, const Bla& arg) {
    int q = 0;
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
    JsonSerializer s(1);
    JsonSerializer& x = s2;
    USerialize a;

//    auto arr = s2.object();
//    auto z = arr.object("asdsaA");
//    auto q = z.array("asfkjsaf");
//    int qrqw;
//    std::optional<int> mqwe = 1;
//    q.value(qrqw);
//    z.value("adasra", qrqw);
//    z.value("z", mqwe);
//    s.value(qrqw);
//    double y;

//    Bla m;
//    z.value("Asd", m);

//    auto ga = s2.object();
//    ga.value("asdas", m);
//    s2.value(y);

    Bla m;
    x.value(m);
    s2.value(m);
    a.value(m);
    return 0;
}
