cmake_minimum_required(VERSION 3.6)
project(nng)

option(NNG_TESTS "" OFF)
option(NNG_ENABLE_NNGCAT "" OFF)

add_subdirectory(nnglib)