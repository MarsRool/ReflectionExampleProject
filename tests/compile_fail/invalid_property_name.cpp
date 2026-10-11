#include "reflection/reflection.h"

struct Record { int actual = 1; };

DECL_REFLECTION_NONINTRUSIVE(Record, &Record::missing)

int main() { return 0; }
