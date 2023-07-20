#include <iostream>
#include "Serializer.h"
#include <cmath>

#include <vector>
#include <unordered_map>

#include "helper/vector.h"
#include "helper/map.h"


struct JsonSerializer : Clio::Serializer<JsonSerializer> {
    CLIO_SERIALIZER(JsonSerializer)

    JsonSerializer(int = 0) {}

protected:
    void beginObject() {}
    void endObject() {}
    void beginArray() {}
    void endArray() {}

    void writeKey(std::string_view) {}
    void write(int) {
        int q = 1;
    }

    std::vector<int> x = { 1, 2, 3 };
};

struct JsonSecondary final : JsonSerializer, Clio::Serializer<JsonSecondary> {
    CLIO_SERIALIZER(JsonSecondary)

protected:
    using JsonSerializer::write;
    void write(double) {}
    std::vector<double> y = { 4, 5, 6 };
};

struct Bla {
    int x;
    std::vector<int> y = { 0, 1, 0 };
};

void serialize(JsonSerializer& s, const Bla& arg) {
    s.value(arg.x);
    s.value(arg.y);
}

void serialize(JsonSecondary& s, const Bla& arg) {
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
    using namespace Clio::detail;

    std::vector<int> ab;
    std::unordered_map<std::string, int> bb;

    JsonSecondary s2;
    JsonSerializer s(1);

    auto arr = s.object();
    auto z = arr.object("asdsaA");
    auto q = z.array("asfkjsaf");
    int qrqw;
    std::optional<int> mqwe = 1;
    q.value(qrqw);
    z.value("adasra", qrqw);
    z.value("z", mqwe);
    s.value(qrqw);
    double y;

    Bla m;
    z.value("Asd", m);

    auto ga = s2.object();
    ga.value("asdas", m);
    s2.value(y);
    return 0;
}
