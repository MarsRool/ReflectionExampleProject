#include "test/reflectedtypes.h"
#include "test/puretypes.h"
#include "test/cross_tu_metadata.h"

namespace cross_tu_test
{

MetadataSnapshot captureFromSecondTu()
{
    return
    {
        detail::captureType<pure::TestObject>(),
        detail::captureType<reflected::TestObject>()
    };
}

} // namespace cross_tu_test
