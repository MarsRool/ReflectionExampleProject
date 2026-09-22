#pragma once
#include "Reflection/reflection.h"

class BaseTestObject
{
public:
    DECL_REFLECTION_BODY(BaseTestObject)

    DECL_PROPERTY_DEFAULT(std::string, name)
    DECL_PROPERTIES_COUNT()
};

class NestedTestObject : public BaseTestObject
{
public:
    DECL_REFLECTION_BODY(NestedTestObject)
    DECL_BASE_CLASS(BaseTestObject)

    DECL_PROPERTY_DEFAULT(bool, isValid)
    DECL_PROPERTIES_COUNT()
};

template <typename T>
class TemplateNestedTestObject : public BaseTestObject
{
public:
    DECL_REFLECTION_BODY(TemplateNestedTestObject<T>)
    DECL_BASE_CLASS(BaseTestObject)

    DECL_PROPERTY_DEFAULT(T, value)
    DECL_PROPERTIES_COUNT()
};

class TestObject : public BaseTestObject
{
public:
    using RealArray = std::array<real, 3>;
    DECL_REFLECTION_BODY(TestObject)
    DECL_BASE_CLASS(BaseTestObject)

    DECL_PROPERTY_DEFAULT(std::size_t, age)
    DECL_PROPERTY_DEFAULT(NestedTestObject, nested)
    DECL_PROPERTY_DEFAULT(TemplateNestedTestObject<int>, templateNested1)
    DECL_PROPERTY_DEFAULT(TemplateNestedTestObject<std::string>, templateNested2)
    DECL_PROPERTY_DEFAULT(std::vector<std::string>, stringArr)
    DECL_PROPERTY_DEFAULT(RealArray, realArr)
    DECL_PROPERTIES_COUNT()
};

TestObject createTestObject();
void serializationTest(const QString& filenameWithoutExt);
void deserializationTest(const QString& filenameWithoutExt);

void equalsTest();

void uniqueStaticMapTest1();
void uniqueStaticMapTest2();
void uniqueStaticMapTest3();

void uniqueStaticArrayTest();

void uniqueStaticHeterogeneousArrayTest();
