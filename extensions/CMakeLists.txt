cmake_minimum_required(VERSION 3.16)

project(reflection_extensions VERSION 0.1.0 LANGUAGES CXX)

# Header-only extensions; prefer the generated reflection header.
set(_extensions_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(_reflection_root "${CMAKE_CURRENT_SOURCE_DIR}/../reflection")
set(_reflection_single_include_dir "${_reflection_root}/single_include")
set(_reflection_include_dir "${_reflection_root}/include")

if(NOT IS_DIRECTORY "${_extensions_include_dir}")
    message(FATAL_ERROR "Extensions include directory is missing: ${_extensions_include_dir}")
endif()

if(EXISTS "${_reflection_single_include_dir}/reflection/reflection.h")
    set(_reflection_include_dirs "${_reflection_single_include_dir}")
elseif(EXISTS "${_reflection_include_dir}/reflection/reflection.h")
    # Support both <reflection.h> and <reflection/...> in source mode.
    set(_reflection_include_dirs
        "${_reflection_include_dir}"
        "${_reflection_include_dir}/reflection"
    )
else()
    message(FATAL_ERROR
        "Reflection headers not found. Expected either "
        "reflection/single_include/reflection/reflection.h or "
        "reflection/include/reflection/reflection.h")
endif()

add_library(reflection_extensions INTERFACE)
add_library(reflection::extensions ALIAS reflection_extensions)

target_include_directories(reflection_extensions INTERFACE
    "$<BUILD_INTERFACE:${_extensions_include_dir}>"
)
foreach(_include_dir IN LISTS _reflection_include_dirs)
    target_include_directories(reflection_extensions INTERFACE
        "$<BUILD_INTERFACE:${_include_dir}>"
    )
endforeach()

target_compile_features(reflection_extensions INTERFACE cxx_std_17)

# JSON extensions may use Qt's QJson* types. Qt belongs to extensions, not reflection.
option(EXTENSIONS_USE_QT "Provide Qt Core to extensions" ON)
if(EXTENSIONS_USE_QT)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
    target_link_libraries(reflection_extensions INTERFACE Qt${QT_VERSION_MAJOR}::Core)
endif()

# Run the header check by default only when extensions is the top-level project.
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

    if(NOT _extensions_headers)
        message(FATAL_ERROR "No extensions headers found in ${_extensions_include_dir}")
    endif()

    set(_extensions_check_dir "${CMAKE_CURRENT_BINARY_DIR}/header_check")
    file(MAKE_DIRECTORY "${_extensions_check_dir}")

    set(_extensions_check_sources)
    foreach(_header IN LISTS _extensions_headers)
        file(RELATIVE_PATH _relative_header "${_extensions_include_dir}" "${_header}")
        string(MAKE_C_IDENTIFIER "${_relative_header}" _header_id)
        string(MD5 _header_hash "${_relative_header}")
        set(_check_source "${_extensions_check_dir}/${_header_id}_${_header_hash}.cpp")

        # Each translation unit includes exactly one public header.
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
