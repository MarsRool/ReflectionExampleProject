#pragma once
#include "Reflection/baseobject.h"

class NestedTestObject : public reflection::BaseObject, public reflection::Reflectable<NestedTestObject>
{
    DECL_REFLECTION_BODY(NestedTestObject, reflection::BaseObject)

    DECL_PROPERTY_DEFAULT(bool, isValid)
    DECL_PROPERTIES_COUNT()

    bool operator==(const ThisClass&) const noexcept = default;
};

template <class T>
class TemplateNestedTestObject : public reflection::BaseObject, public reflection::Reflectable<TemplateNestedTestObject<T>>
{
    DECL_REFLECTION_BODY(TemplateNestedTestObject<T>, reflection::BaseObject)

    DECL_PROPERTY_DEFAULT(T, value)
    DECL_PROPERTIES_COUNT()

    bool operator==(const ThisClass&) const noexcept = default;
};

class TestObject : public reflection::BaseObject, public reflection::Reflectable<TestObject>
{
    using RealArray = std::array<real, 3>;
    DECL_REFLECTION_BODY(TestObject, reflection::BaseObject)

    DECL_PROPERTY_DEFAULT(std::size_t, age)
    DECL_PROPERTY_DEFAULT(NestedTestObject, nested)
    DECL_PROPERTY_DEFAULT(TemplateNestedTestObject<int>, templateNested1)
    DECL_PROPERTY_DEFAULT(TemplateNestedTestObject<std::string>, templateNested2)
    DECL_PROPERTY_DEFAULT(std::vector<std::string>, stringArr)
    DECL_PROPERTY_DEFAULT(RealArray, realArr)
    DECL_PROPERTIES_COUNT()

    bool operator==(const ThisClass&) const noexcept = default;
};

TestObject createTestObject();
void serializationTest(const QString& filenameWithoutExt);
void deserializationTest(const QString& filenameWithoutExt);

void equalsTest();

void uniqueStaticMapTest1();
void uniqueStaticMapTest2();
void uniqueStaticArrayTest();
