cmake_minimum_required(VERSION 3.16)

project(ReflectionExampleProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt code generation support.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_executable(ReflectionExampleProject
    main.cpp
    Test/test.cpp

    Reflection/Extensions/comparisonextension.h
    Reflection/Extensions/extensionstypetraits.h
    Reflection/Extensions/fromjsonextension.h
    Reflection/Extensions/serializationformat.h
    Reflection/Extensions/tojsonextension.h
    Reflection/Extensions/tostringextension.h

    Reflection/Property/basestaticproperty.h
    Reflection/Property/staticproperty.h
    Reflection/Property/staticpropertymap.h
    Reflection/Property/staticpropertymeta.h
    Reflection/Property/staticpropertyproxy.h

    Reflection/Utils/canonicalstaticstring.h
    Reflection/Utils/customexception.h
    Reflection/Utils/filesystem.h
    Reflection/Utils/macroes.h
    Reflection/Utils/statuscode.h
    Reflection/Utils/typetraits.h
    Reflection/Utils/uniqueidcounter.h
    Reflection/Utils/uniquestaticheterogeneousarray.h
    Reflection/Utils/uniquestaticheterogeneousmap.h

    Reflection/reflection.h
    Test/test.h
)

# Compiler warnings
if(MSVC)
    target_compile_options(ReflectionExampleProject PRIVATE
        /W4
        /permissive-
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(ReflectionExampleProject PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -pedantic-errors
        -Wshadow
        -Wnon-virtual-dtor
    )
endif()


# Static analysis
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE
        NAMES clang-tidy
        REQUIRED
    )

    set_target_properties(
        ReflectionExampleProject
        PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}"
    )
endif()

target_include_directories(ReflectionExampleProject
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ReflectionExampleProject
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)
