add_library(clio INTERFACE
    clio/Clio.h
    clio/Serializer.h
    clio/Deserializer.h
    clio/helper/vector.h
    clio/helper/array.h
    clio/helper/map.h
    clio/helper/unordered_map.h
    clio/helper/set.h
    clio/helper/unordered_set.h
)
add_library(libs::clio ALIAS clio)

target_sources(clio PRIVATE
    clio/helper/common.h
)

target_include_directories(clio INTERFACE .)

install(TARGETS clio
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
