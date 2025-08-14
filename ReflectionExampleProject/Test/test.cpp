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
    using StaticValue = const reflection::BaseStaticProperty<reflection::BaseObject>*;

    static constexpr StaticKey key1{ "keyTest1" };
    static constexpr StaticKey key2{ "keyTest2" };
    static constexpr StaticValue value1{ &reflection::BaseObject::nameStaticProperty };
    static constexpr StaticValue value2{ &reflection::BaseObject::typeStaticProperty };

    qDebug() << key1 << key2
             << uniqueStaticMapKeysCount<int, StaticKey, StaticValue>()
             << uniqueStaticMapExists<int, StaticKey, StaticValue, key1>()
             << uniqueStaticMapGetValue<int, StaticKey, StaticValue, key1>()
             << uniqueStaticMapExists<int, StaticKey, StaticValue, key2>()
             << uniqueStaticMapGetValue<int, StaticKey, StaticValue, key2>();

    static constexpr auto _value1 = uniqueStaticMapAdd<int, StaticKey, StaticValue, key1, value1>();
    qDebug() << key1 << key2
             << uniqueStaticMapKeysCount<int, StaticKey, StaticValue>()
             << uniqueStaticMapExists<int, StaticKey, StaticValue, key1>()
             << uniqueStaticMapGetValue<int, StaticKey, StaticValue, key1>()
             << uniqueStaticMapExists<int, StaticKey, StaticValue, key2>()
             << uniqueStaticMapGetValue<int, StaticKey, StaticValue, key2>();

    static constexpr auto _value2 = uniqueStaticMapAdd<int, StaticKey, StaticValue, key1, value2>();
    qDebug() << key1 << key2
             << uniqueStaticMapKeysCount<int, StaticKey, StaticValue>()
             << uniqueStaticMapExists<int, StaticKey, StaticValue, key1>()
             << uniqueStaticMapGetValue<int, StaticKey, StaticValue, key1>()
             << uniqueStaticMapExists<int, StaticKey, StaticValue, key2>()
             << uniqueStaticMapGetValue<int, StaticKey, StaticValue, key2>();

    static constexpr auto _value3 = uniqueStaticMapAdd<int, StaticKey, StaticValue, key2, value2>();
    qDebug() << key1 << key2
             << uniqueStaticMapKeysCount<int, StaticKey, StaticValue>()
             << uniqueStaticMapExists<int, StaticKey, StaticValue, key1>()
             << uniqueStaticMapGetValue<int, StaticKey, StaticValue, key1>()
             << uniqueStaticMapExists<int, StaticKey, StaticValue, key2>()
             << uniqueStaticMapGetValue<int, StaticKey, StaticValue, key2>();
}

void uniqueStaticMapTest2()
{
    using StaticKey = const char[];
    using StaticValue = const char[];

    static constexpr StaticKey key1{ "keyChar1" };
    static constexpr StaticValue value1{ "value1" };
    static constexpr StaticValue value2{ "value2" };

    qDebug() << key1
             << uniqueStaticMapKeysCount<int, StaticKey, StaticValue>()
             << uniqueStaticMapExists<int, StaticKey, StaticValue, key1>()
             << uniqueStaticMapGetValue<int, StaticKey, StaticValue, key1>();

    static constexpr auto _value1 = uniqueStaticMapAdd<int, StaticKey, StaticValue, key1, value1>();
    qDebug() << key1
             << uniqueStaticMapKeysCount<int, StaticKey, StaticValue>()
             << uniqueStaticMapExists<int, StaticKey, StaticValue, key1>()
             << uniqueStaticMapGetValue<int, StaticKey, StaticValue, key1>();

    static constexpr auto _value2 = uniqueStaticMapAdd<int, StaticKey, StaticValue, key1, value2>();
    qDebug() << key1
             << uniqueStaticMapKeysCount<int, StaticKey, StaticValue>()
             << uniqueStaticMapExists<int, StaticKey, StaticValue, key1>()
             << uniqueStaticMapGetValue<int, StaticKey, StaticValue, key1>();
}

void uniqueStaticArrayTest()
{
    using ValueT = const char[];

    static constexpr ValueT value1{ "v1" };
    static constexpr ValueT value2{ "vvvv2" };

    qDebug() << uniqueStaticArrayExists<int, ValueT, 0>()
             << uniqueStaticArrayLength<int, ValueT>()
             << uniqueStaticArrayGetValue<int, ValueT, 0>()
             << uniqueStaticArrayGetValue<int, ValueT, 1>();

    static constexpr auto _value1 = uniqueStaticArrayPushBack<int, ValueT, value1>();
    qDebug() << uniqueStaticArrayExists<int, ValueT, 0>()
             << uniqueStaticArrayLength<int, ValueT>()
             << uniqueStaticArrayGetValue<int, ValueT, 0>()
             << uniqueStaticArrayGetValue<int, ValueT, 1>();

    static constexpr auto _value2 = uniqueStaticArrayPushBack<int, ValueT, value2>();
    qDebug() << uniqueStaticArrayExists<int, ValueT, 0>()
             << uniqueStaticArrayLength<int, ValueT>()
             << uniqueStaticArrayGetValue<int, ValueT, 0>()
             << uniqueStaticArrayGetValue<int, ValueT, 1>();
}
