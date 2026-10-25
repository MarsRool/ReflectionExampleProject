cmake_minimum_required(VERSION 3.18)

# The benchmarks can be configured standalone or included from tests/.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(reflection_compile_benchmarks LANGUAGES CXX)
    include(CTest)
endif()

if(NOT TARGET reflection::reflection)
    add_subdirectory(
        "${CMAKE_CURRENT_SOURCE_DIR}/../reflection"
        "${CMAKE_CURRENT_BINARY_DIR}/reflection"
    )
endif()

if(BUILD_TESTING)
    find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)

    option(REFLECTION_BENCHMARK_STRESS
        "Enable additional 250- and 500-property compile-time stress benchmarks"
        OFF
    )

    set(_benchmark_counts 10 50 100)
    if(REFLECTION_BENCHMARK_STRESS)
        list(APPEND _benchmark_counts 250 500)
    endif()

    foreach(_count IN LISTS _benchmark_counts)
        foreach(_mode IN ITEMS intrusive nonintrusive)
            set(_target "reflection_compile_benchmark_${_mode}_${_count}")
            set(_source "${CMAKE_CURRENT_BINARY_DIR}/generated/${_target}.cpp")
            set(_result "${CMAKE_CURRENT_BINARY_DIR}/results/$<CONFIG>/${_mode}_${_count}.json")

            execute_process(
                COMMAND "${Python3_EXECUTABLE}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/generate_source.py"
                    --mode "${_mode}"
                    --count "${_count}"
                    --output "${_source}"
                RESULT_VARIABLE _generate_result
                ERROR_VARIABLE _generate_error
            )
            if(NOT _generate_result EQUAL 0)
                message(FATAL_ERROR
                    "Failed to generate ${_target}: ${_generate_error}"
                )
            endif()

            # Each benchmark compiles exactly one TU, and is not built by ALL.
            add_library(${_target} OBJECT EXCLUDE_FROM_ALL "${_source}")
            target_link_libraries(${_target} PRIVATE reflection::reflection)
            target_compile_features(${_target} PRIVATE cxx_std_17)
            set_target_properties(${_target} PROPERTIES
                CXX_STANDARD_REQUIRED ON
                CXX_EXTENSIONS OFF
            )

            set(_test "reflection_compile_benchmark.${_mode}.${_count}")
            add_test(NAME "${_test}"
                COMMAND "${Python3_EXECUTABLE}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/measure_compile.py"
                    --build-dir "${CMAKE_BINARY_DIR}"
                    --config "$<CONFIG>"
                    --target "${_target}"
                    --source "${_source}"
                    --result "${_result}"
                    --mode "${_mode}"
                    --count "${_count}"
            )

            # Nested builds of the same tree are unsafe under MSBuild; serial
            # execution also avoids concurrent CPU/memory competition.
            set_tests_properties("${_test}" PROPERTIES
                LABELS "compile-benchmark"
                RUN_SERIAL TRUE
                TIMEOUT 900
            )
        endforeach()
    endforeach()
endif()
