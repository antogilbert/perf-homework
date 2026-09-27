add_executable(spsc spsc.cpp)
target_link_libraries(spsc benchmark::benchmark benchmark::benchmark_main)
