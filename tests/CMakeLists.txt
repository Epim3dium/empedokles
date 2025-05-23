add_executable(
    tests
    test_main.cpp
    core/test_coordinator.cpp
    math/test_geometry.cpp
    math/test_math.cpp
    math/test_transform.cpp
)
# Include FetchContent module
include(FetchContent)

# Fetch Google Test
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
# Ensure FetchContent is set up
FetchContent_MakeAvailable(googletest)
# Enable testing
enable_testing()

include(GoogleTest)
gtest_discover_tests(tests)

include_directories(../src)

target_compile_options(tests PUBLIC -g)


# Link Google Test
target_link_libraries(tests PRIVATE gtest_main empedokles)

