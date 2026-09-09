add_executable(soa soa.cpp)
target_link_libraries(soa benchmark::benchmark benchmark::benchmark_main)

add_executable(soa_struct soa_struct.cpp)
target_link_libraries(soa_struct benchmark::benchmark benchmark::benchmark_main)

add_executable(aos aos.cpp)
target_link_libraries(aos benchmark::benchmark benchmark::benchmark_main)
