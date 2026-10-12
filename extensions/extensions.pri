# Header-only reflection extensions.
# Include this file from a consuming qmake .pro project.

INCLUDEPATH += $$quote($$PWD/include)

EXTENSIONS_REFLECTION_ROOT = $$clean_path($$PWD/../reflection)
EXTENSIONS_REFLECTION_SINGLE_DIR = $$EXTENSIONS_REFLECTION_ROOT/single_include
EXTENSIONS_REFLECTION_INCLUDE_DIR = $$EXTENSIONS_REFLECTION_ROOT/include

exists($$EXTENSIONS_REFLECTION_SINGLE_DIR/reflection/reflection.h) {
    INCLUDEPATH += $$quote($$EXTENSIONS_REFLECTION_SINGLE_DIR)
} else:exists($$EXTENSIONS_REFLECTION_INCLUDE_DIR/reflection/reflection.h) {
    # Support both <reflection.h> and <reflection/...> in source mode.
    INCLUDEPATH += $$quote($$EXTENSIONS_REFLECTION_INCLUDE_DIR)
    INCLUDEPATH += $$quote($$EXTENSIONS_REFLECTION_INCLUDE_DIR/reflection)
} else {
    error("Reflection headers not found (single_include or include)")
}

CONFIG += c++17

# Required when the JSON extensions use Qt's QJson* types.
QT += core
lessThan(QT_MAJOR_VERSION, 6): error("Reflection extensions require Qt 6 or newer")
