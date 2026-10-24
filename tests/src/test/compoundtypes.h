#pragma once

#include "test/puretypes.h"
#include "test/reflectedtypes.h"

namespace compound
{

struct CompoundTestObject
{
    pure::TestObject pureTestObject;
    reflected::TestObject reflectedTestObject;
};

REFLECTION_NONINTRUSIVE(CompoundTestObject,
    &CompoundTestObject::pureTestObject,
    &CompoundTestObject::reflectedTestObject)

} // namespace compound
