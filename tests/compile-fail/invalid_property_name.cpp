#include "reflection/reflection.h"

struct Record
{
    int actual = 1;
};

REFLECTION_NONINTRUSIVE(Record, &Record::missing)

static_assert(reflection::StaticPropertyMeta<Record>::size() == 1);
