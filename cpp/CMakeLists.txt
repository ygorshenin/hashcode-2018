cmake_minimum_required(VERSION 3.6)

project(SelfDrivingRides)

option(USE_PPROF OFF)
option(USE_ASAN OFF)

add_definitions(-DHOME)
if (USE_PPROF)
  add_definitions(-DUSE_PPROF)
endif()

add_compile_options(
  "-std=c++11"
  "-Wall"
)

if (USE_ASAN)
  add_compile_options(
    "-fno-omit-frame-pointer"
    "-fsanitize=address"
  )
endif()

add_executable(solution solution.cpp)

if (USE_PPROF)
  target_link_libraries(solution "-lprofiler")
endif()

if (USE_ASAN)
  target_link_libraries(solution "-fsanitize=address")
endif()
