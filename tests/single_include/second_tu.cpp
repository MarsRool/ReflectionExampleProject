#include "shared.h"

namespace single_include_smoke
{

using Meta = reflection::StaticPropertyMeta<IntrusiveRecord>;

static_assert(Meta::size() == 3);
static_assert(Meta::contains<scoreKey>());

const void* propertiesAddressFromSecondTu()
{
    return &Meta::properties;
}

int scoreFromSecondTu()
{
    const IntrusiveRecord object{};
    return Meta::at<scoreKey>()->get(object);
}

} // namespace single_include_smoke
