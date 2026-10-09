cmake_minimum_required(VERSION 3.16)

project(reflection VERSION 0.1.0 LANGUAGES CXX)

# Pure header-only library with no Qt dependency.
add_library(reflection INTERFACE)
add_library(reflection::reflection ALIAS reflection)

target_include_directories(reflection INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(reflection INTERFACE cxx_std_17)

# Do not build development checks when reflection is used via add_subdirectory().
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(_reflection_header_check_default ON)
else()
    set(_reflection_header_check_default OFF)
endif()

option(REFLECTION_BUILD_HEADER_CHECK
    "Compile every reflection header independently"
    ${_reflection_header_check_default}
)

if(REFLECTION_BUILD_HEADER_CHECK)
    file(GLOB_RECURSE _reflection_headers CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx"
    )

    if(NOT _reflection_headers)
        message(FATAL_ERROR "No reflection headers found in include/")
    endif()

    set(_reflection_check_sources)
    foreach(_header IN LISTS _reflection_headers)
        file(RELATIVE_PATH _relative_header
            "${CMAKE_CURRENT_SOURCE_DIR}/include" "${_header}")
        string(MAKE_C_IDENTIFIER "${_relative_header}" _header_id)
        set(_check_source "${CMAKE_CURRENT_BINARY_DIR}/header_check/${_header_id}.cpp")

        # Each translation unit must compile using only its own includes.
        file(WRITE "${_check_source}" "#include <${_relative_header}>\n")
        list(APPEND _reflection_check_sources "${_check_source}")
    endforeach()

    set(_reflection_check_main "${CMAKE_CURRENT_BINARY_DIR}/header_check/main.cpp")
    file(WRITE "${_reflection_check_main}" "int main() { return 0; }\n")

    add_executable(reflection_header_check ${_reflection_check_sources} "${_reflection_check_main}")
    target_link_libraries(reflection_header_check PRIVATE reflection::reflection)
    set_target_properties(reflection_header_check PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endif()
