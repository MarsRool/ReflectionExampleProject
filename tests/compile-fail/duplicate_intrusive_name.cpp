#include "reflection/reflection.h"

struct Record
{
    REFLECTION_BEGIN(Record)
    REFLECTION_PROPERTY(int, first)
    REFLECTION_PROPERTY_CUSTOM(int, second, "first", 2)
    REFLECTION_END()
};

static_assert(reflection::StaticPropertyMeta<Record>::size() == 2);
