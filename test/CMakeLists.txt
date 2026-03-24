cmake_minimum_required(VERSION 4.0.2)
project(data-validator)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)

FetchContent_MakeAvailable(googletest)

add_compile_options(-Wall -Werror -Wextra -Wpedantic)

include_directories(../lib)

add_library(csv STATIC
    csv_impl.c
)

enable_testing()

add_executable(run_tests
  test_csv.cc
)

target_link_libraries(run_tests
		PRIVATE
				csv
				GTest::gtest_main
)

set_source_files_properties(csv_impl.c PROPERTIES
    COMPILE_OPTIONS "-Wno-unused-function"
)

include(GoogleTest)
gtest_discover_tests(
	run_tests
)
