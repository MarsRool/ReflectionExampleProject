#include "Test/test.h"

int main(int argc, char* argv[])
{
    serializationTest("D:/test");
    deserializationTest("D:/test");

    equalsTest();

    // uniqueStaticMapTest1();
    // uniqueStaticMapTest2();
    // uniqueStaticArrayTest();

    return 0;
}
