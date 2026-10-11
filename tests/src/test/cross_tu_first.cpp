#include "test/puretypes.h"
#include "test/reflectedtypes.h"
#include "test/cross_tu_metadata.h"

namespace cross_tu_test
{

MetadataSnapshot captureFromFirstTu()
{
    // Force independent compile-time evaluation in this translation unit.
    constexpr MetadataSnapshot snapshot
    {
        detail::captureType<pure::TestObject, detail::ageKey, 10>(),
        detail::captureType<reflected::TestObject, detail::ageKey, 10>(),
        detail::captureType<pure::TemplateNestedTestObject<int>, detail::valueKey, 2>(),
        detail::captureType<reflected::TemplateNestedTestObject<int>, detail::valueKey, 2>()
    };
    return snapshot;
}

} // namespace cross_tu_test
