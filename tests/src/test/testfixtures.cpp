#include "test/test.h"

compound::CompoundTestObject createCompoundTestObject()
{
    compound::CompoundTestObject compoundTestObject;
    compoundTestObject.pureTestObject = createTestObject<pure::TestObject>();
    compoundTestObject.reflectedTestObject = createTestObject<reflected::TestObject>();
    return compoundTestObject;
}
