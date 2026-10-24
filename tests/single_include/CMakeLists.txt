# Copy only the generated header into an isolated include tree. The smoke test
# must not acquire modular reflection headers through a transitive dependency.
set(_single_header
    "${CMAKE_CURRENT_SOURCE_DIR}/../../reflection/single_include/reflection/reflection.h"
)

if(NOT EXISTS "${_single_header}")
    message(FATAL_ERROR
        "Generated reflection header not found: ${_single_header}. "
        "Run python tools/amalgamate.py before configuring the tests."
    )
endif()

set(_isolated_include_dir "${CMAKE_CURRENT_BINARY_DIR}/isolated_include")
configure_file(
    "${_single_header}"
    "${_isolated_include_dir}/reflection/reflection.h"
    COPYONLY
)

add_executable(reflection_single_include_smoke
    smoke.cpp
    second_tu.cpp
)

target_include_directories(reflection_single_include_smoke PRIVATE
    "${_isolated_include_dir}"
)
target_compile_features(reflection_single_include_smoke PRIVATE cxx_std_17)
set_target_properties(reflection_single_include_smoke PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Deliberately do not link reflection::reflection, reflection::extensions or
# GoogleTest. The header must work as a standalone C++17 dependency.
add_test(NAME reflection_single_include_smoke
    COMMAND reflection_single_include_smoke
)

find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
add_test(NAME reflection_single_include_generated
    COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../tools/amalgamate.py" --check
)

set_tests_properties(
    reflection_single_include_smoke
    reflection_single_include_generated
    PROPERTIES LABELS "single-include"
)
