cmake_minimum_required(VERSION 3.5)

project(clio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(clio
    main.cpp
    Clio.h
    Serializer.h
    Deserializer.h
    helper/vector.h
    helper/map.h
    helper/unordered_map.h
    helper/set.h
    helper/unordered_set.h
    helper/common.h
)

install(TARGETS clio
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
