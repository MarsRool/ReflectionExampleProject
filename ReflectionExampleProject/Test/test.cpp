#include "Test/test.h"

#include "Reflection/Extensions/comparisonextension.h"
#include "Reflection/Extensions/fromjsonextension.h"
#include "Reflection/Extensions/tojsonextension.h"
#include "Reflection/Extensions/tostringextension.h"

using namespace reflection;

TestObject createTestObject()
{
    TestObject test;

    test.name = "Name";
    test.anotherName = "Another name";

    test.nestedValue.name = "Nested value";
    test.nested.isValid = false;

    test.templateNestedValue.name = "Template Nested Value";
    test.templateNestedValue.value = 123.456f;

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

void serializationTest(const QString& filenameWithoutExt)
{
    const auto test = createTestObject();

    const auto sc = extensions::save(test,
        SerializationFormat::Json, filenameWithoutExt);

    if (isGood(sc))
    {
        qDebug() << "serializationTest passed";
    }
    else
    {
        qCritical() << "serializationTest failed";
    }
}

void deserializationTest(const QString &filenameWithoutExt)
{
    const auto test = createTestObject();

    TestObject loadedTest;
    CHECK_SC(extensions::load(loadedTest,
        SerializationFormat::Json, filenameWithoutExt))

    if (extensions::equal(test, loadedTest))
    {
        qDebug() << "deserializationTest passed";
    }
    else
    {
        qCritical() << "deserializationTest failed";
    }
}

void stringConversionTest()
{
    const auto test = createTestObject();

    const auto testString = extensions::convertToString(test);

    static constexpr const char expectedString[] = "\"TestObject\":\n{ \"type\": \"TestObject\",\n\"_base_BaseTestObject\":\n{ \"type\": \"BaseTestObject\",\n\"name\": \"Name\",\n },\n\"_base_AnotherBaseTestObject\":\n{ \"type\": \"AnotherBaseTestObject\",\n\"anotherName\": \"Another name\",\n },\n\"age\": 157,\n\"nested\": \"NestedTestObject\":\n{ \"type\": \"NestedTestObject\",\n\"_base_BaseTestObject\":\n{ \"type\": \"BaseTestObject\",\n\"name\": \"Nested test\",\n },\n\"isValid\": true,\n },\n\"templateNested1\": \"TemplateNestedTestObject<T>\":\n{ \"type\": \"TemplateNestedTestObject<T>\",\n\"_base_BaseTestObject\":\n{ \"type\": \"BaseTestObject\",\n\"name\": \"Template nested 1st\",\n },\n\"value\": -137,\n },\n\"templateNested2\": \"TemplateNestedTestObject<T>\":\n{ \"type\": \"TemplateNestedTestObject<T>\",\n\"_base_BaseTestObject\":\n{ \"type\": \"BaseTestObject\",\n\"name\": \"Template nested 2nd\",\n },\n\"value\": \"some value\",\n },\n\"nestedPtr\": \"NestedTestObject\":\n{ \"type\": \"NestedTestObject\",\n\"_base_BaseTestObject\":\n{ \"type\": \"BaseTestObject\",\n\"name\": \"Nested value\",\n },\n\"isValid\": false,\n },\n\"templateNestedPtr\": \"TemplateNestedTestObject<T>\":\n{ \"type\": \"TemplateNestedTestObject<T>\",\n\"_base_BaseTestObject\":\n{ \"type\": \"BaseTestObject\",\n\"name\": \"Template Nested Value\",\n },\n\"value\": 123.456001,\n },\n\"stringArr\": [ \"asdf\", \"fdsa\" ],\n\"realArr\": [ 1.750000, -1651.130000, 179.000000 ],\n }";

    if (expectedString == testString)
    {
        qDebug() << "stringConversionTest passed";
    }
    else
    {
        qCritical() << "stringConversionTest failed";
    }
}

void equalsTest()
{
    const auto test = createTestObject();

    auto test2{ test };

    const bool equals1 = extensions::equal(test, test2);

    if (!equals1)
    {
        qCritical() << "equalsTest failed: copy is NOT equal to original object";
    }

    test2.age += 15;

    const bool equals2 = extensions::equal(test, test2);

    if (equals2)
    {
        qCritical() << "equalsTest failed: objects are equal after change";
    }

    if (equals1 && !equals2)
    {
        qDebug() << "equalsTest passed";
    }
}

void uniqueStaticPropertyMapTest()
{
    using StaticKey = const char[];

    static constexpr StaticKey nonexistentKey{ "nonexistent" };
    static constexpr StaticKey existentKey1{ "name" };
    static constexpr auto existentKey2{ CanonicalStaticString<existentKey1>::value };

    static_assert(existentKey1 != existentKey2);

    static_assert(BaseTestObject::staticPropertyMap.empty() == false);
    static_assert(BaseTestObject::staticPropertyMap.size() == 2);

    static_assert(BaseTestObject::staticPropertyMap.contains<nonexistentKey>() == false);
    static_assert(BaseTestObject::staticPropertyMap.at<nonexistentKey>() == nullptr);

    static_assert(BaseTestObject::staticPropertyMap.contains<existentKey1>() == true);
    static_assert(BaseTestObject::staticPropertyMap.at<existentKey1>() != nullptr);

    static_assert(BaseTestObject::staticPropertyMap.contains<existentKey2>() == true);
    static_assert(BaseTestObject::staticPropertyMap.at<existentKey2>() != nullptr);

    static_assert(BaseTestObject::staticPropertyMap.at<existentKey1>()
        == BaseTestObject::staticPropertyMap.at<existentKey2>());
}

void uniqueStaticHeterogeneousMapTest1()
{
    struct OuterT{};
    using StaticKey = const char[];
    using Meta = StaticPropertyClassMeta<BaseTestObject>;

    static constexpr StaticKey key1{ "keyTest1" };
    static constexpr StaticKey key2{ "keyTest2" };
    static constexpr StaticKey key3{ "keyTest3" };
    static constexpr auto valuePtr1{ Meta::get<&BaseTestObject::name>() };
    static constexpr auto valuePtr2{ Meta::get<&BaseTestObject::type>() };
    using ValueT1 = decltype(valuePtr1);
    using ValueT2 = decltype(valuePtr2);

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 0);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == nullptr);

    uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, ValueT1, key1, valuePtr1>([]{});

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == valuePtr1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == nullptr);

    uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, ValueT2, key1, valuePtr2>([]{});

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == valuePtr1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == nullptr);

    uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, ValueT2, key2, valuePtr2>([]{});

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 2);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == valuePtr1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == valuePtr2);

    std::size_t foundCount1 = 0;
    std::size_t foundCount2 = 0;
    std::size_t foundCount3 = 0;

    uniqueStaticHeterogeneousMapDoForKey<OuterT, StaticKey, key1>([]{},
        [&foundCount1](auto constKey, auto constValue)
    {
        static_assert(decltype(constKey)::value == key1);
        static_assert(decltype(constValue)::value == valuePtr1);
        ++foundCount1;
    });
    uniqueStaticHeterogeneousMapDoForKey<OuterT, StaticKey, key2>([]{},
        [&foundCount2](auto constKey, auto constValue)
    {
        static_assert(decltype(constKey)::value == key2);
        static_assert(decltype(constValue)::value == valuePtr2);
        ++foundCount2;
    });
    uniqueStaticHeterogeneousMapDoForKey<OuterT, StaticKey, key3>([]{},
        [&foundCount3](auto, auto)
    {
        static_assert(false);
        ++foundCount3;
    });

    if (foundCount1 == 1 && foundCount2 == 1 && foundCount3 == 0)
    {
        qDebug() << "uniqueStaticHeterogeneousMapTest1 passed";
    }
    else
    {
        qCritical() << "uniqueStaticHeterogeneousMapTest1 failed";
    }
}

void uniqueStaticHeterogeneousMapTest2()
{
    struct OuterT{};
    using StaticKey = const char[];
    using StaticValue1 = const char[];
    using StaticValue2 = std::size_t;

    static constexpr StaticKey key1{ "keyChar1" };
    static constexpr StaticValue1 value1{ "value1" };
    static constexpr StaticValue2 value2{ 42 };

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 0);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == nullptr);

    uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, StaticValue1, key1, value1>([]{});

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == value1);

    uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, StaticValue2, key1, value2>([]{});

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == value1);
}

void uniqueStaticHeterogeneousArrayTest()
{
    struct OuterT{};
    using Value1Type = const char[];
    using Value2Type = std::int32_t;
    using Value3Type = const std::int32_t*;

    static constexpr Value1Type value1{ "test value" };
    static constexpr Value2Type value2{ 42 };
    static constexpr Value3Type value3{ &value2 };

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 0);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 3>([]{}) == nullptr);

    uniqueStaticHeterogeneousArrayPushBack<OuterT, Value1Type, value1>([]{});

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == value1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == nullptr);

    uniqueStaticHeterogeneousArrayPushBack<OuterT, Value2Type, value2>([]{});

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 2);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == value1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == value2);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == nullptr);

    uniqueStaticHeterogeneousArrayPushBack<OuterT, Value3Type, value3>([]{});

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 3);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == value1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == value2);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == value3);

    static constexpr auto valueRead1 = uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{});
    static constexpr auto valueRead2 = uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{});
    static constexpr auto valueRead3 = uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{});

    static_assert(std::is_same_v<decltype(valueRead1), std::add_const_t<std::decay_t<Value1Type>>>);
    static_assert(std::is_same_v<decltype(valueRead2), std::add_const_t<Value2Type>>);
    static_assert(std::is_same_v<decltype(valueRead3), std::add_const_t<Value3Type>>);
}
