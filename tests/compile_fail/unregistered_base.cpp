#include "reflection/reflection.h"

struct UnregisteredBase {};
struct Derived : UnregisteredBase { int value = 1; };

DECL_REFLECTION_NONINTRUSIVE(Derived, &Derived::value)
DECL_REFLECTION_INHERITANCE_NONINTRUSIVE(Derived, UnregisteredBase)

int main() { return 0; }
