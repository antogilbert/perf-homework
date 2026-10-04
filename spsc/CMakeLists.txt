enable_testing()

add_library(spsc spsc.cpp)

add_executable(tests tests.cpp)

target_link_libraries(tests PRIVATE GTest::gtest_main)

add_executable(bench bench.cpp)
target_link_libraries(bench
    spsc
    benchmark::benchmark
    benchmark::benchmark_main
)
