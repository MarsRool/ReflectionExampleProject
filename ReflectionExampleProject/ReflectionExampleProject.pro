QT = core widgets

CONFIG += c++20 precompile_header

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

PRECOMPILED_HEADER = pch.h

HEADERS += \
    Reflection/Property/Instance/baseproperty.h \
    Reflection/Property/Instance/customproperty.h \
    Reflection/Property/Instance/property.h \
    Reflection/Property/Instance/propertymap.h \
    Reflection/Property/Static/basestaticproperty.h \
    Reflection/Property/Static/staticproperty.h \
    Reflection/Property/Static/staticpropertymap.h \
    Reflection/Property/Static/staticpropertyproxy.h \
    Reflection/Utils/aliasinfo.h \
    Reflection/Utils/valueutils.h \
    Reflection/baseobject.h \
    Reflection/reflectable.h \
    Reflection/reflection.h \
    Reflection/serializationformat.h \
    Shared/checkmacroes.h \
    Shared/customexception.h \
    Shared/filesystem.h \
    Shared/macroes.h \
    Shared/statuscode.h \
    Shared/typetester.h \
    Shared/uniqueidcounter.h \
    Shared/uniquestaticarray.h \
    Shared/uniquestaticmap.h \
    Test/test.h

SOURCES += \
        Reflection/Property/Instance/baseproperty.cpp \
        Reflection/baseobject.cpp \
        Shared/customexception.cpp \
        Shared/filesystem.cpp \
        Test/test.cpp \
        main.cpp
