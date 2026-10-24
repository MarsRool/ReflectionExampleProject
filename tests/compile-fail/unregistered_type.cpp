#include "reflection/reflection.h"

struct Record {};

static_assert(reflection::StaticPropertyMeta<Record>::size() == 0);
