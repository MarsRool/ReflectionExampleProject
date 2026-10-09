#pragma once
#include "Reflection/reflection.h"
#include "Test/compoundtypes.h"

template <typename T>
T createTestObject()
{
    T test;

    test.name = "Name";
    test.anotherName = "Another name";

    test.nestedStorage->name = "Nested value";
    test.nestedStorage->isValid = false;

    test.templateNestedStorage->name = "Template Nested Value";
    test.templateNestedStorage->value = 123.456f;

    test.age = 157;

    test.nested.name = "Nested test";
    test.nested.isValid = true;

    test.templateNested1.name = "Template nested 1st";
    test.templateNested1.value = -137;

    test.templateNested2.name = "Template nested 2nd";
    test.templateNested2.value = "some value";

    test.stringArr.push_back("asdf");
    test.stringArr.push_back("fdsa");

    test.realArr[0] = 1.75;
    test.realArr[1] = -1651.13;
    test.realArr[2] = 179;

    return test;
}

compound::CompoundTestObject createCompoundTestObject();

void serializationTest(const QString& filenameWithoutExt);
void deserializationTest(const QString& filenameWithoutExt);

void stringConversionTest();

void equalsTest();

void uniqueStaticPropertyMapTest();

void uniqueStaticHeterogeneousMapTest1();
void uniqueStaticHeterogeneousMapTest2();

void uniqueStaticHeterogeneousArrayTest();
