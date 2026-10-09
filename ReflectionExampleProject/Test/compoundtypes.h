#pragma once
#include "Test/puretypes.h"
#include "Test/reflectedtypes.h"

namespace compound
{

struct CompoundTestObject
{
    DECL_REFLECTION_BODY(CompoundTestObject)

    DECL_PROPERTY_DEFAULT(pure::TestObject, pureTestObject)
    DECL_PROPERTY_DEFAULT(reflected::TestObject, reflectedTestObject)
};

// Note, non-intrusive macro is NOT necessary
// it's just a check, that reflection can handle both (intrusive and non-intrusive)
// on the same types without breaking or any side effects

DECL_REFLECTION_NONINTRUSIVE(CompoundTestObject,
    &CompoundTestObject::pureTestObject,
    &CompoundTestObject::reflectedTestObject)

} // namespace compound
