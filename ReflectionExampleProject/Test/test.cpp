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

    const auto sc = reflection::save(test,
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
    CHECK_SC(reflection::load(loadedTest,
        reflection::SerializationFormat::Json, filenameWithoutExt))

    if (TestObject::staticPropertyMap.equals(test, loadedTest))
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

    const bool equals1 = TestObject::staticPropertyMap.equals(test, test2);

    if (!equals1)
    {
        qCritical() << "equalsTest failed: copy is NOT equal to original object";
    }

    test2.age += 15;

    const bool equals2 = TestObject::staticPropertyMap.equals(test, test2);

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
    static constexpr auto existentKey2{ BaseTestObject::nameStaticPropertyName };
    static constexpr auto existentKey3{ CanonicalStaticStringT<BaseTestObject::nameStaticPropertyName>::value };

    static_assert(existentKey1 != existentKey2 && existentKey2 != existentKey3);

    static_assert(BaseTestObject::staticPropertyMap.empty() == false);
    static_assert(BaseTestObject::staticPropertyMap.size() == 2);

    static_assert(BaseTestObject::staticPropertyMap.contains<nonexistentKey>() == false);
    static_assert(BaseTestObject::staticPropertyMap.at<nonexistentKey>() == nullptr);

    static_assert(BaseTestObject::staticPropertyMap.contains<existentKey1>() == true);
    static_assert(BaseTestObject::staticPropertyMap.at<existentKey1>() != nullptr);

    static_assert(BaseTestObject::staticPropertyMap.contains<existentKey2>() == true);
    static_assert(BaseTestObject::staticPropertyMap.at<existentKey2>() != nullptr);

    static_assert(BaseTestObject::staticPropertyMap.contains<existentKey3>() == true);
    static_assert(BaseTestObject::staticPropertyMap.at<existentKey3>() != nullptr);

    static_assert(BaseTestObject::staticPropertyMap.at<existentKey1>() == BaseTestObject::staticPropertyMap.at<existentKey2>()
                  && BaseTestObject::staticPropertyMap.at<existentKey2>() == BaseTestObject::staticPropertyMap.at<existentKey3>());
}

void uniqueStaticMapTest1()
{
    struct OuterT{};
    using StaticKey = const char[];
    using StaticPropertyPtr = const reflection::BaseStaticProperty<BaseTestObject>* const;
    using StaticPropertyDPtr = StaticPropertyPtr*;

    static constexpr StaticKey key1{ "keyTest1" };
    static constexpr StaticKey key2{ "keyTest2" };
    static constexpr StaticPropertyPtr valuePtr1{ &BaseTestObject::nameStaticProperty };
    static constexpr StaticPropertyPtr valuePtr2{ &BaseTestObject::typeStaticProperty };
    static constexpr StaticPropertyDPtr valueDPtr1{ &valuePtr1 };
    static constexpr StaticPropertyDPtr valueDPtr2{ &valuePtr2 };

    static_assert(uniqueStaticMapKeysCount<OuterT, StaticKey, StaticPropertyDPtr>([]{}) == 0);
    static_assert(uniqueStaticMapExists<OuterT, StaticKey, StaticPropertyDPtr, key1>([]{}) == false);
    static_assert(uniqueStaticMapGetValue<OuterT, StaticKey, StaticPropertyDPtr, key1>([]{}) == nullptr);
    static_assert(uniqueStaticMapExists<OuterT, StaticKey, StaticPropertyDPtr, key2>([]{}) == false);
    static_assert(uniqueStaticMapGetValue<OuterT, StaticKey, StaticPropertyDPtr, key2>([]{}) == nullptr);

    static constexpr auto _value1 = uniqueStaticMapAdd<OuterT, StaticKey, StaticPropertyDPtr, key1, valueDPtr1>([]{});
    (void)_value1;

    static_assert(uniqueStaticMapKeysCount<OuterT, StaticKey, StaticPropertyDPtr>([]{}) == 1);
    static_assert(uniqueStaticMapExists<OuterT, StaticKey, StaticPropertyDPtr, key1>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<OuterT, StaticKey, StaticPropertyDPtr, key1>([]{}) == valueDPtr1);
    static_assert(uniqueStaticMapExists<OuterT, StaticKey, StaticPropertyDPtr, key2>([]{}) == false);
    static_assert(uniqueStaticMapGetValue<OuterT, StaticKey, StaticPropertyDPtr, key2>([]{}) == nullptr);

    static constexpr auto _value2 = uniqueStaticMapAdd<OuterT, StaticKey, StaticPropertyDPtr, key1, valueDPtr2>([]{});
    (void)_value2;

    static_assert(uniqueStaticMapKeysCount<OuterT, StaticKey, StaticPropertyDPtr>([]{}) == 1);
    static_assert(uniqueStaticMapExists<OuterT, StaticKey, StaticPropertyDPtr, key1>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<OuterT, StaticKey, StaticPropertyDPtr, key1>([]{}) == valueDPtr1);
    static_assert(uniqueStaticMapExists<OuterT, StaticKey, StaticPropertyDPtr, key2>([]{}) == false);
    static_assert(uniqueStaticMapGetValue<OuterT, StaticKey, StaticPropertyDPtr, key2>([]{}) == nullptr);

    static constexpr auto _value3 = uniqueStaticMapAdd<OuterT, StaticKey, StaticPropertyDPtr, key2, valueDPtr2>([]{});
    (void)_value3;

    static_assert(uniqueStaticMapKeysCount<OuterT, StaticKey, StaticPropertyDPtr>([]{}) == 2);
    static_assert(uniqueStaticMapExists<OuterT, StaticKey, StaticPropertyDPtr, key1>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<OuterT, StaticKey, StaticPropertyDPtr, key1>([]{}) == valueDPtr1);
    static_assert(uniqueStaticMapExists<OuterT, StaticKey, StaticPropertyDPtr, key2>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<OuterT, StaticKey, StaticPropertyDPtr, key2>([]{}) == valueDPtr2);
}

void uniqueStaticMapTest2()
{
    struct OuterT{};
    using StaticKey = const char[];
    using StaticValue = const char[];

    static constexpr StaticKey key1{ "keyChar1" };
    static constexpr StaticValue value1{ "value1" };
    static constexpr StaticValue value2{ "value2" };

    static_assert(uniqueStaticMapKeysCount<OuterT, StaticKey, StaticValue>([]{}) == 0);
    static_assert(uniqueStaticMapExists<OuterT, StaticKey, StaticValue, key1>([]{}) == false);
    static_assert(uniqueStaticMapGetValue<OuterT, StaticKey, StaticValue, key1>([]{}) == nullptr);

    static constexpr auto _value1 = uniqueStaticMapAdd<OuterT, StaticKey, StaticValue, key1, value1>([]{});
    (void)_value1;

    static_assert(uniqueStaticMapKeysCount<OuterT, StaticKey, StaticValue>([]{}) == 1);
    static_assert(uniqueStaticMapExists<OuterT, StaticKey, StaticValue, key1>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<OuterT, StaticKey, StaticValue, key1>([]{}) == value1);

    static constexpr auto _value2 = uniqueStaticMapAdd<OuterT, StaticKey, StaticValue, key1, value2>([]{});
    (void)_value2;

    static_assert(uniqueStaticMapKeysCount<OuterT, StaticKey, StaticValue>([]{}) == 1);
    static_assert(uniqueStaticMapExists<OuterT, StaticKey, StaticValue, key1>([]{}) == true);
    static_assert(uniqueStaticMapGetValue<OuterT, StaticKey, StaticValue, key1>([]{}) == value1);
}

void uniqueStaticHeterogeneousMapTest1()
{
    struct OuterT{};
    using StaticKey = const char[];

    static constexpr StaticKey key1{ "keyTest1" };
    static constexpr StaticKey key2{ "keyTest2" };
    static constexpr auto valuePtr1{ &BaseTestObject::nameStaticProperty };
    static constexpr auto valuePtr2{ &BaseTestObject::typeStaticProperty };
    using ValueT1 = decltype(valuePtr1);
    using ValueT2 = decltype(valuePtr2);

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 0);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == nullptr);

    static constexpr auto _value1 = uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, ValueT1, key1, valuePtr1>([]{});
    (void)_value1;

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == valuePtr1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == nullptr);

    static constexpr auto _value2 = uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, ValueT2, key1, valuePtr2>([]{});
    (void)_value2;

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == valuePtr1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == nullptr);

    static constexpr auto _value3 = uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, ValueT2, key2, valuePtr2>([]{});
    (void)_value3;

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 2);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == valuePtr1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == valuePtr2);
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

    static constexpr auto _value1 = uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, StaticValue1, key1, value1>([]{});
    (void)_value1;

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == value1);

    static constexpr auto _value2 = uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, StaticValue2, key1, value2>([]{});
    (void)_value2;

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == value1);
}

void uniqueStaticArrayTest()
{
    struct OuterT{};
    using ValueT = const char[];

    static constexpr ValueT value1{ "v1" };
    static constexpr ValueT value2{ "vvvv2" };

    static_assert(uniqueStaticArrayExists<OuterT, ValueT, 0>([]{}) == false);
    static_assert(uniqueStaticArrayLength<OuterT, ValueT>([]{}) == 0);
    static_assert(uniqueStaticArrayGetValue<OuterT, ValueT, 0>([]{}) == nullptr);
    static_assert(uniqueStaticArrayGetValue<OuterT, ValueT, 1>([]{}) == nullptr);

    static constexpr auto _value1 = uniqueStaticArrayPushBack<OuterT, ValueT, value1>([]{});
    (void)_value1;

    static_assert(uniqueStaticArrayExists<OuterT, ValueT, 0>([]{}) == true);
    static_assert(uniqueStaticArrayLength<OuterT, ValueT>([]{}) == 1);
    static_assert(uniqueStaticArrayGetValue<OuterT, ValueT, 0>([]{}) == value1);
    static_assert(uniqueStaticArrayGetValue<OuterT, ValueT, 1>([]{}) == nullptr);

    static constexpr auto _value2 = uniqueStaticArrayPushBack<OuterT, ValueT, value2>([]{});
    (void)_value2;

    static_assert(uniqueStaticArrayExists<OuterT, ValueT, 0>([]{}) == true);
    static_assert(uniqueStaticArrayLength<OuterT, ValueT>([]{}) == 2);
    static_assert(uniqueStaticArrayGetValue<OuterT, ValueT, 0>([]{}) == value1);
    static_assert(uniqueStaticArrayGetValue<OuterT, ValueT, 1>([]{}) == value2);
}

void uniqueStaticHeterogeneousArrayTest()
{
    struct OuterT{};
    using Value1T = const char[];
    using Value2T = std::int32_t;
    using Value3T = const std::int32_t*;

    static constexpr Value1T value1{ "test value" };
    static constexpr Value2T value2{ 42 };
    static constexpr Value3T value3{ &value2 };

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 0);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 3>([]{}) == nullptr);

    static constexpr auto _value1 = uniqueStaticHeterogeneousArrayPushBack<OuterT, Value1T, value1>([]{});
    (void)_value1;

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == value1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == nullptr);

    static constexpr auto _value2 = uniqueStaticHeterogeneousArrayPushBack<OuterT, Value2T, value2>([]{});
    (void)_value2;

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 2);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == value1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == value2);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == nullptr);

    static constexpr auto _value3 = uniqueStaticHeterogeneousArrayPushBack<OuterT, Value3T, value3>([]{});
    (void)_value3;

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 3);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == value1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == value2);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == value3);

    static constexpr auto valueRead1 = uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{});
    static constexpr auto valueRead2 = uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{});
    static constexpr auto valueRead3 = uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{});

    static_assert(std::is_same_v<decltype(valueRead1), std::add_const_t<std::decay_t<Value1T>>>);
    static_assert(std::is_same_v<decltype(valueRead2), std::add_const_t<Value2T>>);
    static_assert(std::is_same_v<decltype(valueRead3), std::add_const_t<Value3T>>);
}
