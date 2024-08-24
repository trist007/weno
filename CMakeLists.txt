cmake_minimum_required(VERSION 3.26)
project(weno)

set(CMAKE_CXX_STANDARD 17)

find_library(TRANTOR_LIB
              NAMES trantor
              PATHS /usr/local/lib64)
add_executable(weno main.cpp)
target_link_libraries(weno PRIVATE ${TRANTOR_LIB})
