cmake_minimum_required(VERSION 3.16)
project(BreadSliceCoreTests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bread_slice_core STATIC
  ../src/bsc_estop.cpp
  ../src/bsc_parse.cpp
  ../src/bsc_watchdog.cpp
  fakes/Arduino.cpp)
target_include_directories(bread_slice_core PUBLIC ../src fakes)
target_compile_definitions(bread_slice_core PUBLIC BSC_HOST_TEST)
target_compile_options(bread_slice_core PRIVATE -Wall -Wextra -Werror)

enable_testing()
foreach(name test_estop test_watchdog test_parse test_status_led)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PRIVATE bread_slice_core)
  target_compile_options(${name} PRIVATE -Wall -Wextra)
  add_test(NAME ${name} COMMAND ${name})
endforeach()
