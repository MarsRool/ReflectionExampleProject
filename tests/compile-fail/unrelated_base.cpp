#include "reflection/reflection.h"

struct Unrelated {};
REFLECTION_NONINTRUSIVE(Unrelated)

struct Record
{
    int value = 1;
};

REFLECTION_NONINTRUSIVE(Record, &Record::value)
REFLECTION_BASE_CLASSES_NONINTRUSIVE(Record, Unrelated)

static_assert(reflection::StaticPropertyMeta<Record>::size() == 2);
