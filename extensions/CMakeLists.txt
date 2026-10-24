cmake_minimum_required(VERSION 3.16)

project(reflection_extensions VERSION 0.1.0 LANGUAGES CXX)

set(_extensions_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(_repository_root "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Support standalone configuration and consumption by a parent CMake project.
if(NOT TARGET reflection::reflection)
    add_subdirectory("${_repository_root}/reflection"
        "${CMAKE_CURRENT_BINARY_DIR}/reflection")
endif()

# Prefer an existing package; otherwise use the pinned Git submodule.
if(NOT TARGET nlohmann_json::nlohmann_json)
    find_package(nlohmann_json 3.12.0 CONFIG QUIET)
endif()

if(NOT TARGET nlohmann_json::nlohmann_json)
    set(_json_submodule "${_repository_root}/third_party/json")
    if(NOT EXISTS "${_json_submodule}/CMakeLists.txt")
        message(FATAL_ERROR
            "nlohmann/json 3.12.0 is required. Install its CMake package or run: "
            "git submodule update --init --recursive (third_party/json)")
    endif()

    set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
    set(JSON_Install OFF CACHE BOOL "" FORCE)
    add_subdirectory("${_json_submodule}" "${CMAKE_CURRENT_BINARY_DIR}/nlohmann_json"
        EXCLUDE_FROM_ALL)
endif()

add_library(reflection_extensions INTERFACE)
add_library(reflection::extensions ALIAS reflection_extensions)

target_include_directories(reflection_extensions INTERFACE
    "$<BUILD_INTERFACE:${_extensions_include_dir}>"
)
target_link_libraries(reflection_extensions INTERFACE
    reflection::reflection
    nlohmann_json::nlohmann_json
)
target_compile_features(reflection_extensions INTERFACE cxx_std_17)

# Check self-contained public headers only in the standalone extensions project.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(_extensions_header_check_default ON)
else()
    set(_extensions_header_check_default OFF)
endif()

option(EXTENSIONS_BUILD_HEADER_CHECK
    "Compile every extensions header independently"
    ${_extensions_header_check_default}
)

if(EXTENSIONS_BUILD_HEADER_CHECK)
    file(GLOB_RECURSE _extensions_headers CONFIGURE_DEPENDS
        "${_extensions_include_dir}/*.h"
        "${_extensions_include_dir}/*.hpp"
        "${_extensions_include_dir}/*.hxx"
    )

    set(_extensions_check_dir "${CMAKE_CURRENT_BINARY_DIR}/header_check")
    file(MAKE_DIRECTORY "${_extensions_check_dir}")

    set(_extensions_check_sources)
    foreach(_header IN LISTS _extensions_headers)
        file(RELATIVE_PATH _relative_header "${_extensions_include_dir}" "${_header}")
        string(MAKE_C_IDENTIFIER "${_relative_header}" _header_id)
        string(MD5 _header_hash "${_relative_header}")
        set(_check_source "${_extensions_check_dir}/${_header_id}_${_header_hash}.cpp")
        file(WRITE "${_check_source}" "#include <${_relative_header}>\n")
        list(APPEND _extensions_check_sources "${_check_source}")
    endforeach()

    set(_extensions_check_main "${_extensions_check_dir}/main.cpp")
    file(WRITE "${_extensions_check_main}" "int main() { return 0; }\n")

    add_executable(extensions_header_check
        ${_extensions_check_sources}
        "${_extensions_check_main}"
    )
    target_link_libraries(extensions_header_check PRIVATE reflection::extensions)
    set_target_properties(extensions_header_check PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endif()
