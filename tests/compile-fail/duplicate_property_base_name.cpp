#include "reflection/reflection.h"

struct Base {};
REFLECTION_NONINTRUSIVE(Base)

struct Record : Base
{
    int value = 1;
};

REFLECTION_CUSTOM_NONINTRUSIVE(Record, "Record", "_base_Base", &Record::value)
REFLECTION_BASE_CLASSES_NONINTRUSIVE(Record, Base)

static_assert(reflection::StaticPropertyMeta<Record>::size() == 2);
