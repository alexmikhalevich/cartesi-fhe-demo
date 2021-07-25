cmake_minimum_required(VERSION 3.0)

project(string_cap_fhe_test)

set(CMAKE_CXX_FLAGS "-std=c++20 -ggdb -O0")

include_directories(
    ${CMAKE_SOURCE_DIR}/third-party/tfhe/src/include
    ${CMAKE_SOURCE_DIR}/include)
link_directories(
    ${CMAKE_BINARY_DIR}/third-party/tfhe/src/build/libtfhe)

add_subdirectory(third-party/tfhe/src)

set(LIB_SOURCES src/string_cap_char.cc)
set(LIB_FHE_SOURCES src/string_cap_char_tfhe.cc)
set(EXE_SOURCES src/main.cc)

add_library(string_cap_char ${LIB_SOURCES})
add_library(string_cap_char_tfhe ${LIB_FHE_SOURCES})
add_executable(string_cap_char_fhe_test ${EXE_SOURCES})

target_link_libraries(string_cap_char_tfhe
    string_cap_char
    tfhe-nayuki-portable)

target_link_libraries(string_cap_char_fhe_test
    string_cap_char
    string_cap_char_tfhe
    tfhe-nayuki-portable)
