cmake_minimum_required(VERSION 2.6)
project(benchmark)
add_executable(benchmark)
find_package(Boost COMPONENTS filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(benchmark gtest gtest_main ${Boost_FILESYSTEM_LIBRARY})
