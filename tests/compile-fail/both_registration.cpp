#include "reflection/reflection.h"

struct Record
{
    REFLECTION_BEGIN(Record)
    REFLECTION_PROPERTY(int, value)
    REFLECTION_END()
};

REFLECTION_NONINTRUSIVE(Record, &Record::value)

static_assert(reflection::StaticPropertyMeta<Record>::size() == 1);
