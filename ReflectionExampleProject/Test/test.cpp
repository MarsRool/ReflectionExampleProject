#include "Test/test.h"

TestObject createTestObject()
{
    TestObject test;
    test.age = 157;
    test.name = "Markus";
    test.nested.name = "Nested test";
    test.nested.isValid = true;
    test.templateNested1.value = -137;
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

    const auto sc = test.reflection::Reflectable<TestObject>::save(
        reflection::SerializationFormat::Json, filenameWithoutExt);

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
    CHECK_SC(loadedTest.reflection::Reflectable<TestObject>::load(
        reflection::SerializationFormat::Json, filenameWithoutExt))

    if (test.reflection::Reflectable<TestObject>::getPropertyMap().equals(
            loadedTest.reflection::Reflectable<TestObject>::getPropertyMap()))
    {
        qDebug() << "deserializationTest passed";
    }
    else
    {
        qCritical() << "deserializationTest failed";
    }
}

void equalsTest()
{
    const auto test = createTestObject();

    auto test2{ test };

    const bool equals1 = test.reflection::Reflectable<TestObject>::getPropertyMap().equals(
        test2.reflection::Reflectable<TestObject>::getPropertyMap());

    if (!equals1)
    {
        qCritical() << "equalsTest failed: copy is NOT equal to original object";
    }

    test2.age += 15;

    const bool equals2 = test.reflection::Reflectable<TestObject>::getPropertyMap().equals(
        test2.reflection::Reflectable<TestObject>::getPropertyMap());

    if (equals2)
    {
        qCritical() << "equalsTest failed: objects are equal after change";
    }

    if (equals1 && !equals2)
    {
        qDebug() << "equalsTest passed";
    }
}

void uniqueStaticMapTest1()
{
    using StaticKey = const char[];
    using StaticPropertyPtr = const reflection::BaseStaticProperty<reflection::BaseObject>* const;
    using StaticPropertyDPtr = StaticPropertyPtr*;

    static constexpr StaticKey key1{ "keyTest1" };
    static constexpr StaticKey key2{ "keyTest2" };
    static constexpr StaticPropertyPtr valuePtr1{ &reflection::BaseObject::nameStaticProperty };
    static constexpr StaticPropertyPtr valuePtr2{ &reflection::BaseObject::typeStaticProperty };
    static constexpr StaticPropertyDPtr valueDPtr1{ &valuePtr1 };
    static constexpr StaticPropertyDPtr valueDPtr2{ &valuePtr2 };

    static_assert(uniqueStaticMapKeysCount<int, StaticKey, StaticPropertyDPtr>([]{}) == 0);
    static_assert(uniqueStaticMapExists<int, StaticKey, StaticPropertyDPtr, key1>([]{}) == false);
    static_assert(uniqueStaticMapGetValue<int, StaticKey, StaticPropertyDPtr, key1>([]{}) == nullptr);
    static_assert(uniqueStaticMapExists<int, StaticKey, StaticPropertyDPtr, key2>([]{}) == false);
    static_assert(uniqueStaticMapGetValue<int, StaticKey, StaticPropertyDPtr, key2>([]{}) == nullptr);

    static constexpr auto _value1 = uniqueStaticMapAdd<int, StaticKey, StaticPropertyDPtr, key1, valueDPtr1>([]{});

    static_assert(uniqueStaticMapKeysCount<int, StaticKey, StaticPropertyDPtr>([]{}) == 1);
    static_assert(uniqueStaticMapExists<int, StaticKey, StaticPropertyDPtr, key1>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<int, StaticKey, StaticPropertyDPtr, key1>([]{}) == valueDPtr1);
    static_assert(uniqueStaticMapExists<int, StaticKey, StaticPropertyDPtr, key2>([]{}) == false);
    static_assert(uniqueStaticMapGetValue<int, StaticKey, StaticPropertyDPtr, key2>([]{}) == nullptr);

    static constexpr auto _value2 = uniqueStaticMapAdd<int, StaticKey, StaticPropertyDPtr, key1, valueDPtr2>([]{});

    static_assert(uniqueStaticMapKeysCount<int, StaticKey, StaticPropertyDPtr>([]{}) == 1);
    static_assert(uniqueStaticMapExists<int, StaticKey, StaticPropertyDPtr, key1>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<int, StaticKey, StaticPropertyDPtr, key1>([]{}) == valueDPtr1);
    static_assert(uniqueStaticMapExists<int, StaticKey, StaticPropertyDPtr, key2>([]{}) == false);
    static_assert(uniqueStaticMapGetValue<int, StaticKey, StaticPropertyDPtr, key2>([]{}) == nullptr);

    static constexpr auto _value3 = uniqueStaticMapAdd<int, StaticKey, StaticPropertyDPtr, key2, valueDPtr2>([]{});

    static_assert(uniqueStaticMapKeysCount<int, StaticKey, StaticPropertyDPtr>([]{}) == 2);
    static_assert(uniqueStaticMapExists<int, StaticKey, StaticPropertyDPtr, key1>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<int, StaticKey, StaticPropertyDPtr, key1>([]{}) == valueDPtr1);
    static_assert(uniqueStaticMapExists<int, StaticKey, StaticPropertyDPtr, key2>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<int, StaticKey, StaticPropertyDPtr, key2>([]{}) == valueDPtr2);
}

void uniqueStaticMapTest2()
{
    using StaticKey = const char[];
    using StaticValue = const char[];

    static constexpr StaticKey key1{ "keyChar1" };
    static constexpr StaticValue value1{ "value1" };
    static constexpr StaticValue value2{ "value2" };

    static_assert(uniqueStaticMapKeysCount<int, StaticKey, StaticValue>([]{}) == 0);
    static_assert(uniqueStaticMapExists<int, StaticKey, StaticValue, key1>([]{}) == false);
    static_assert(uniqueStaticMapGetValue<int, StaticKey, StaticValue, key1>([]{}) == nullptr);

    static constexpr auto _value1 = uniqueStaticMapAdd<int, StaticKey, StaticValue, key1, value1>([]{});

    static_assert(uniqueStaticMapKeysCount<int, StaticKey, StaticValue>([]{}) == 1);
    static_assert(uniqueStaticMapExists<int, StaticKey, StaticValue, key1>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<int, StaticKey, StaticValue, key1>([]{}) == value1);

    static constexpr auto _value2 = uniqueStaticMapAdd<int, StaticKey, StaticValue, key1, value2>([]{});

    static_assert(uniqueStaticMapKeysCount<int, StaticKey, StaticValue>([]{}) == 1);
    static_assert(uniqueStaticMapExists<int, StaticKey, StaticValue, key1>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<int, StaticKey, StaticValue, key1>([]{}) == value1);
}

void uniqueStaticArrayTest()
{
    using ValueT = const char[];

    static constexpr ValueT value1{ "v1" };
    static constexpr ValueT value2{ "vvvv2" };

    static_assert(uniqueStaticArrayExists<int, ValueT, 0>([]{}) == false);
    static_assert(uniqueStaticArrayLength<int, ValueT>([]{}) == 0);
    static_assert(uniqueStaticArrayGetValue<int, ValueT, 0>([]{}) == nullptr);
    static_assert(uniqueStaticArrayGetValue<int, ValueT, 1>([]{}) == nullptr);

    static constexpr auto _value1 = uniqueStaticArrayPushBack<int, ValueT, value1>([]{});

    static_assert(uniqueStaticArrayExists<int, ValueT, 0>([]{}) == true);
    static_assert(uniqueStaticArrayLength<int, ValueT>([]{}) == 1);
    static_assert(uniqueStaticArrayGetValue<int, ValueT, 0>([]{}) == value1);
    static_assert(uniqueStaticArrayGetValue<int, ValueT, 1>([]{}) == nullptr);

    static constexpr auto _value2 = uniqueStaticArrayPushBack<int, ValueT, value2>([]{});

    static_assert(uniqueStaticArrayExists<int, ValueT, 0>([]{}) == true);
    static_assert(uniqueStaticArrayLength<int, ValueT>([]{}) == 2);
    static_assert(uniqueStaticArrayGetValue<int, ValueT, 0>([]{}) == value1);
    static_assert(uniqueStaticArrayGetValue<int, ValueT, 1>([]{}) == value2);
}
