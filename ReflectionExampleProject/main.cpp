#include "Test/test.h"

int main()
{
    serializationTest("D:/test");
    deserializationTest("D:/test");

    stringConversionTest();

    equalsTest();

    uniqueStaticPropertyMapTest();

    uniqueStaticHeterogeneousMapTest1();
    uniqueStaticHeterogeneousMapTest2();

    uniqueStaticHeterogeneousArrayTest();

    return 0;
}
