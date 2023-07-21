# Clio

## Description
Named after the muse of history, the library is a header only lighweight serialization/deserialization interface.
It is written in and is requiring C++17 (at least).

## Usage
Define your implementation and pull in the interface classes.

For serialization:
```
struct MySerializer : Clio::Serializer<MySerializer> {
    CLIO_SERIALIZER(MySerializer)

    // Your public interface

protected:
    // Primitive types or classes that are supported:
    void write(bool val);
    void write(int val);
    void write(unsigned int val);
    void write(const std::string& val);

    // When a key needs to be serialized (will be called only after beginObject())
    void writeKey(std::string_view k);

    // Begins/ends an object
    void beginObject();
    void endObject();
    // Begin/ends an array
    void beginArray();
    void endArray();
    // More implementation details
};
```
Or for deserialization:
```
struct MyDeserializer : Clio::Deserializer<MyDeserializer> {
    CLIO_DESERIALIZER(MyDeserializer)

    // Your public interface

protected:
    // Primitive types or classes that are directly supported:
    void read(bool& val);
    void read(int& val);
    void read(unsigned int& val);
    void read(std::string& val);

    // Key management will be called only after beginObject()  
    const std::string& peekKey() const noexcept; // Will be called to get the next key, for example if you're using a helper like for std::map
    bool hasKey(std::string_view key) const noexcept;  // Checks if key is available
    void readKey(std::string_view key); // Reads the key and should advance the internal state (if any)

    // Begins/ends an object
    void beginObject();
    void endObject();
    // Begin/ends an array
    void beginArray();
    void endArray();

    // Should return the size of the pending array or object (size for an object is the number of available keys)
    std::size_t size() const noexcept;

    // More implementation details
};
```

## Extending the interface

The interface can be extended externaly by defining a global `serialize()` function. The resolution what function to call (either `serialize()` or `Class::write` is done through ADL, which allows to specialize the implementation for specific (De)Serializer or type.
**Note:** For serialization of primitive types the interface will refuse to call a `write()` method that's not an exact match, i.e. it will not allow an implicit conversion.
Inheriting a (de)serializer can be done trivially by pulling the appropriate interface along. For example:
```
struct MyExtendedSerializer : Clio::Serializer<MyExtendedSerializer>, protected MySerializer {
    CLIO_SERIALIZER(MyExtendedSerializer)

    using MySerializer::write; // Pull base class overloads if necessary (i.e. if you extend the types that are supported)
    void write(const MyVerySpecialType&);
};
```

A fully generic extension is possible through the usage of a template `serialize()` function (as done in the helpers), for example such an implementation for a `std::map` may look like:
```
template <typename Interface, typename ... Args>
std::enable_if_t<Clio::is_serializer_v<Interface>> serialize(Interface&, const std::map<Args...>&) {
    // Implementation
}
```

Few helpers are provided in the `helper` subdir to facilitate serialzation of `std::map`, `std::set`, `std::unordered_map`, `std::unordered_set` and `std::vector`. Include the like-named headers as needed.
