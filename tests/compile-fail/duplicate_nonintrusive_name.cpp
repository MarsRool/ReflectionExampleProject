#include "reflection/reflection.h"

struct Record
{
    int first = 1;
    int second = 2;
};

REFLECTION_CUSTOM_NONINTRUSIVE(Record, "Record", "duplicate, duplicate",
    &Record::first, &Record::second)

static_assert(reflection::StaticPropertyMeta<Record>::size() == 2);
