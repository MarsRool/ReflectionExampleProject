QT = core widgets

CONFIG += c++17 precompile_header

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Reflection/Extensions/comparisonextension.h \
    Reflection/Extensions/fromjsonextension.h \
    Reflection/Extensions/serializationformat.h \
    Reflection/Extensions/tojsonextension.h \
    Reflection/Extensions/tostringextension.h \
    Reflection/Property/basestaticproperty.h \
    Reflection/Property/staticproperty.h \
    Reflection/Property/staticpropertymap.h \
    Reflection/Property/staticpropertyproxy.h \
    Reflection/Utils/canonicalstaticstring.h \
    Reflection/Utils/customexception.h \
    Reflection/Utils/filesystem.h \
    Reflection/Utils/macroes.h \
    Reflection/Utils/statuscode.h \
    Reflection/Utils/typetraits.h \
    Reflection/Utils/uniqueidcounter.h \
    Reflection/Utils/uniquestaticarray.h \
    Reflection/Utils/uniquestaticheterogeneousarray.h \
    Reflection/Utils/uniquestaticheterogeneousmap.h \
    Reflection/Utils/uniquestaticmap.h \
    Reflection/reflection.h \
    Test/test.h

SOURCES += \
        Reflection/Utils/customexception.cpp \
        Reflection/Utils/filesystem.cpp \
        Test/test.cpp \
        main.cpp
