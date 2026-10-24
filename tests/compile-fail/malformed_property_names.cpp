#include "reflection/reflection.h"

constexpr char names[] = "&Record::first,";
using Parser = reflection::CanonicalPropertyNameParser<names>;

static_assert(Parser::namesCount == 1);
