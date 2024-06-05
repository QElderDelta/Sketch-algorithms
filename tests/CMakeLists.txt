include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.11.0
)

FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(distribution_test shard_data_test.cpp ../shard_data.cpp)
target_compile_definitions(distribution_test PRIVATE VEC_DISABLED__)
target_include_directories(distribution_test PRIVATE ../)
target_link_libraries(distribution_test PRIVATE GTest::GTest)
add_test(distribution_test distribution_test)
