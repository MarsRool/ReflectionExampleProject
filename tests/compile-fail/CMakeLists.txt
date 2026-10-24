# Compile-fail tests use the selected CMake generator/toolchain to invoke the
# compiler, instead of spawning cl.exe (or another compiler) directly.

set(_core_compile_fail_cases
    "both_registration|A type cannot have both intrusive and non-intrusive reflection"
    "duplicate_intrusive_name|Duplicate reflection property name"
    "duplicate_nonintrusive_name|Duplicate reflection property name"
    "duplicate_base_name|Duplicate reflection property name"
    "duplicate_property_base_name|Duplicate reflection property name"
    "unrelated_base|Invalid registered base class"
    "self_base|Invalid registered base class"
    "unregistered_base|No reflection registration found for this type"
    "unregistered_type|No reflection registration found for this type"
    "invalid_property_name|missing"
    "malformed_property_names|invalid propertyNames"
    "mismatched_registration|property names count doesn't match value pointers count"
)

set(_extension_compile_fail_cases
    "unsupported_pointer|JSON conversion does not support pointers"
    "unsupported_map|Unsupported type for JSON serialization"
    "unsupported_initializer_list_deserialization|JSON deserialization requires insertable containers or writable sized ranges"
)

foreach(_group IN ITEMS core extension)
    foreach(_case IN LISTS _${_group}_compile_fail_cases)
        string(REPLACE "|" ";" _parts "${_case}")
        list(GET _parts 0 _name)
        list(GET _parts 1 _expected_diagnostic)

        set(_target "reflection_compile_fail_${_name}")
        add_library(${_target} OBJECT EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/${_name}.cpp")
        target_compile_features(${_target} PRIVATE cxx_std_17)
        set_target_properties(${_target} PROPERTIES
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
        )

        if(_group STREQUAL "extension")
            target_link_libraries(${_target} PRIVATE reflection::extensions)
        else()
            target_link_libraries(${_target} PRIVATE reflection::reflection)
        endif()

        set(_test_name "reflection_compile_fail.${_name}")
        add_test(NAME "${_test_name}"
            COMMAND "${CMAKE_COMMAND}"
                "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
                "-DBUILD_CONFIG=$<CONFIG>"
                "-DTARGET_NAME=${_target}"
                "-DEXPECTED_DIAGNOSTIC=${_expected_diagnostic}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/check.cmake"
        )
        # Concurrent builds of the same CMake tree can collide under MSBuild.
        set_tests_properties("${_test_name}" PROPERTIES
            LABELS "compile-fail"
            RESOURCE_LOCK "reflection_compile_fail_build"
        )
    endforeach()
endforeach()
