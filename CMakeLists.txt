cmake_minimum_required(VERSION 3.19)

project(sketch_experiment CXX)

set(CMAKE_CXX_STANDARD 20)

enable_testing()
add_compile_options(-Wall -Wextra -pedantic -Werror -march=native)

add_executable(sketch main.cpp shard_data.cpp segment.cpp estimators.cpp common.cpp baseline_common.cpp)
target_include_directories(sketch PRIVATE sketch/include sketch/include/blaze)
target_compile_definitions(sketch PRIVATE VEC_DISABLED__)

add_executable(presence_checkers_comparison presence_checkers_comparison.cpp shard_data.cpp segment.cpp presence_checkers.cpp common.cpp baseline_common.cpp)
target_include_directories(presence_checkers_comparison PRIVATE sketch/include sketch/include/blaze)
target_compile_definitions(presence_checkers_comparison PRIVATE VEC_DISABLED__)

add_subdirectory(tests)
