#include "reflection/reflection.h"

inline constexpr char malformedNames[] = "&Record::first,";
using Parser = reflection::CanonicalPropertyNameParser<malformedNames>;
static_assert(Parser::count == 1);

int main() { return 0; }
