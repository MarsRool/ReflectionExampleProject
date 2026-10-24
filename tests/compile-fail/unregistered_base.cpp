#include "reflection/reflection.h"

struct Base
{
    int value = 1;
};

struct Record : Base
{
    int own = 2;
};

REFLECTION_NONINTRUSIVE(Record, &Record::own)
REFLECTION_BASE_CLASSES_NONINTRUSIVE(Record, Base)

// Traversal must instantiate reflection metadata for the unregistered base.
void visit()
{
    reflection::StaticPropertyTraversal::forEach<Record>([](auto) {});
}
