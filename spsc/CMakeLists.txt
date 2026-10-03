enable_testing()

add_executable(spsc spsc.cpp)
# target_link_libraries(spsc benchmark::benchmark benchmark::benchmark_main)

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE GTest::gtest_main)

include(GoogleTest)
