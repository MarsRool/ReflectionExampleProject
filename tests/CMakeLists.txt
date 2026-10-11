cmake_minimum_required(VERSION 3.18)

project(reflection_tests LANGUAGES CXX)

# Keep the existing standalone and parent-project workflows.
if(NOT TARGET reflection::extensions)
    add_subdirectory(
        "${CMAKE_CURRENT_SOURCE_DIR}/../extensions"
        "${CMAKE_CURRENT_BINARY_DIR}/extensions"
    )
endif()

set(_googletest_root "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/googletest")
if(NOT TARGET GTest::gtest)
    if(NOT EXISTS "${_googletest_root}/CMakeLists.txt")
        message(FATAL_ERROR
            "GoogleTest submodule was not found at ${_googletest_root}. "
            "Run: git submodule update --init --recursive"
        )
    endif()

    add_subdirectory(
        "${_googletest_root}"
        "${CMAKE_CURRENT_BINARY_DIR}/googletest"
        EXCLUDE_FROM_ALL
    )

    if(TARGET gtest)
        set_property(TARGET gtest PROPERTY CXX_CLANG_TIDY "")
    endif()
endif()

file(GLOB_RECURSE _test_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

if(NOT _test_sources)
    message(FATAL_ERROR "No test .cpp files found in tests/src")
endif()

add_executable(reflection_tests ${_test_sources})

target_include_directories(reflection_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(reflection_tests PRIVATE
    reflection::extensions
    GTest::gtest
)

target_compile_features(reflection_tests PRIVATE cxx_std_17)
set_target_properties(reflection_tests PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# The test executable provides its own main() in src/main.cpp.
# Enable testing at the root CMakeLists.txt if built as a subproject.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(CTest)
endif()

if(BUILD_TESTING)
    include(GoogleTest)
    # Delay discovery: on Windows, Qt DLLs may not be on PATH during build.
    gtest_discover_tests(reflection_tests DISCOVERY_MODE PRE_TEST)
endif()
