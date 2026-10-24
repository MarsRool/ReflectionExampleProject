#include "reflection/reflection.h"

struct Base
{
    int value = 1;
};

REFLECTION_NONINTRUSIVE(Base, &Base::value)

struct Record : Base
{
    int own = 2;
};

REFLECTION_NONINTRUSIVE(Record, &Record::own)
REFLECTION_BASE_CLASSES_NONINTRUSIVE(Record, Base, Base)

static_assert(reflection::StaticPropertyMeta<Record>::size() == 3);
