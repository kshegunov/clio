cmake_minimum_required(VERSION 3.5)

project(clio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CLIO_BUILD_TESTS "Clio: Build tests" OFF)

add_subdirectory(src)
if(CLIO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
