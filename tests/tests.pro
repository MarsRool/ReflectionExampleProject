# GoogleTest runner, backed by the same pinned submodule as CMake.
TEMPLATE = app
TARGET = reflection_tests

QT = core
CONFIG += console c++17
CONFIG -= app_bundle

include($$PWD/../extensions/extensions.pri)
INCLUDEPATH += $$quote($$PWD/src)

GOOGLETEST_ROOT = $$clean_path($$PWD/../third_party/googletest/googletest)
!exists($$GOOGLETEST_ROOT/src/gtest-all.cc): error("GoogleTest submodule is missing; run git submodule update --init --recursive")

INCLUDEPATH += $$quote($$GOOGLETEST_ROOT)
INCLUDEPATH += $$quote($$GOOGLETEST_ROOT/include)

SOURCES += $$files(src/*.cpp, true)
SOURCES += $$quote($$GOOGLETEST_ROOT/src/gtest-all.cc)

HEADERS += $$files(src/*.h, true)
