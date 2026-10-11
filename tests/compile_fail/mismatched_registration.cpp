#include "reflection/reflection.h"

struct Record
{
    DECL_REFLECTION_BODY(Record)
    int first = 1;
};

inline constexpr char twoNames[] = "&Record::first, &Record::second";
static_assert(reflection::StaticPropertyMeta<Record>::defineStaticPropertyMulti<
    twoNames, &Record::first>());

int main() { return 0; }
