#include "Test/test.h"

int main()
{
    serializationTest("D:/test");
    deserializationTest("D:/test");

    stringConversionTest();

    equalsTest();

    uniqueStaticPropertyMapTest();

    uniqueStaticMapTest1();
    uniqueStaticMapTest2();

    uniqueStaticHeterogeneousMapTest1();
    uniqueStaticHeterogeneousMapTest2();

    uniqueStaticArrayTest();

    uniqueStaticHeterogeneousArrayTest();

    return 0;
}
