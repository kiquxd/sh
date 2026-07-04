cmake_minimum_required(VERSION 3.14)
project(ShellTests C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(myshell src/main.c)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(shell_tests tests/tests.cpp)
target_link_libraries(shell_tests PRIVATE Catch2::Catch2WithMain)

target_compile_definitions(shell_tests PRIVATE MYSHELL_PATH="$<TARGET_FILE:myshell>")

enable_testing()
add_test(NAME ShellIntegrationTests COMMAND shell_tests)