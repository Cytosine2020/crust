cmake_minimum_required(VERSION 3.0)

project(crust)

set(CMAKE_CXX_STANDARD 11)

if (MSVC)
    add_compile_options(/W4 /EHs- /EHc- /GR-)
else()
    add_compile_options(-Wall -Wextra -fno-exceptions -fno-rtti)
endif()

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -g3 -fno-omit-frame-pointer -D __DEBUG__")
endif()

include_directories(include)

add_executable(test test/test.cpp)
