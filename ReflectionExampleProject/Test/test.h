#pragma once
#include "Reflection/reflection.h"

struct BaseTestObject
{
    DECL_REFLECTION_BODY(BaseTestObject)

    DECL_PROPERTY_DEFAULT(std::string, name)
    DECL_PROPERTIES_COUNT()
};

struct NestedTestObject : public BaseTestObject
{
    DECL_REFLECTION_BODY(NestedTestObject)
    DECL_BASE_CLASS(BaseTestObject)

    DECL_PROPERTY_DEFAULT(bool, isValid)
    DECL_PROPERTIES_COUNT()
};

template <typename T>
struct TemplateNestedTestObject : public BaseTestObject
{
    DECL_REFLECTION_BODY(TemplateNestedTestObject<T>)
    DECL_BASE_CLASS(BaseTestObject)

    DECL_PROPERTY_DEFAULT(T, value)
    DECL_PROPERTIES_COUNT()
};

struct AnotherBaseTestObject
{
    DECL_REFLECTION_BODY(AnotherBaseTestObject)

    DECL_PROPERTY_DEFAULT(std::string, anotherName)
    DECL_PROPERTIES_COUNT()
};

struct TestObject : public BaseTestObject, public AnotherBaseTestObject
{
    DECL_REFLECTION_BODY(TestObject)
    DECL_BASE_CLASS(BaseTestObject)
    DECL_BASE_CLASS(AnotherBaseTestObject)

    using DoubleArray = std::array<double, 3>;

    NestedTestObject nestedValue;
    TemplateNestedTestObject<float> templateNestedValue;

    DECL_PROPERTY_DEFAULT(std::size_t, age)
    DECL_PROPERTY_DEFAULT(NestedTestObject, nested)
    DECL_PROPERTY_DEFAULT(TemplateNestedTestObject<int>, templateNested1)
    DECL_PROPERTY_DEFAULT(TemplateNestedTestObject<std::string>, templateNested2)
    DECL_PROPERTY_INIT(NestedTestObject*, nestedPtr, &nestedValue)
    DECL_PROPERTY_INIT(TemplateNestedTestObject<float>*, templateNestedPtr, &templateNestedValue)
    DECL_PROPERTY_DEFAULT(std::vector<std::string>, stringArr)
    DECL_PROPERTY_DEFAULT(DoubleArray, realArr)
    DECL_PROPERTIES_COUNT()
};

TestObject createTestObject();

void serializationTest(const QString& filenameWithoutExt);
void deserializationTest(const QString& filenameWithoutExt);

void stringConversionTest();

void equalsTest();

void uniqueStaticPropertyMapTest();

void uniqueStaticMapTest1();
void uniqueStaticMapTest2();

void uniqueStaticHeterogeneousMapTest1();
void uniqueStaticHeterogeneousMapTest2();

void uniqueStaticArrayTest();

void uniqueStaticHeterogeneousArrayTest();
