cmake_minimum_required(VERSION 3.1)

project(crust)

add_compile_definitions(GTEST_DONT_DEFINE_TEST)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if (result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif ()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if (result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif ()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
    ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
    EXCLUDE_FROM_ALL)

if (NOT MSVC)
  add_compile_options(-Wall -Wextra -fno-exceptions -fno-rtti)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} \
    -ggdb -g3 -fno-omit-frame-pointer")
else ()
  add_compile_options(/W 3 /sdl /EHr /GR-)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")
endif ()

IF(CMAKE_BUILD_TYPE MATCHES RELEASE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
ENDIF(CMAKE_BUILD_TYPE MATCHES RELEASE)

include_directories(include)

file(GLOB TEST_SRC "test/*.hpp" "test/*.cpp")

add_executable(test-cxx11 ${TEST_SRC})
target_compile_features(test-cxx11 PUBLIC cxx_std_11)
target_link_libraries(test-cxx11 gtest_main)

add_executable(test-cxx14 ${TEST_SRC})
target_compile_features(test-cxx14 PUBLIC cxx_std_14)
target_link_libraries(test-cxx14 gtest_main)

add_executable(test-cxx17 ${TEST_SRC})
target_compile_features(test-cxx17 PUBLIC cxx_std_17)
target_link_libraries(test-cxx17 gtest_main)

add_executable(test-cxx20 ${TEST_SRC})
target_compile_features(test-cxx20 PUBLIC cxx_std_20)
target_link_libraries(test-cxx20 gtest_main)
