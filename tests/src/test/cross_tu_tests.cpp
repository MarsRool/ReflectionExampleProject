#include <gtest/gtest.h>

#include "test/cross_tu_metadata.h"

namespace
{

void expectSameMetadata(const cross_tu_test::TypeSnapshot& first,
                        const cross_tu_test::TypeSnapshot& second)
{
    ASSERT_FALSE(first.propertyNames.empty());
    EXPECT_EQ(first.propertyCount, first.propertyNames.size());
    EXPECT_EQ(second.propertyCount, second.propertyNames.size());
    EXPECT_EQ(first.propertyNames, second.propertyNames);

    ASSERT_NE(first.propertiesAddress, nullptr);
    ASSERT_NE(first.nestedPropertyAddress, nullptr);
    EXPECT_EQ(first.propertiesAddress, second.propertiesAddress);
    EXPECT_EQ(first.nestedPropertyAddress, second.nestedPropertyAddress);
}

} // namespace

TEST(ReflectionCrossTU, MetadataAndPropertyAddressesAreStable)
{
    const auto first = cross_tu_test::captureFromFirstTu();
    const auto second = cross_tu_test::captureFromSecondTu();

    expectSameMetadata(first.pure, second.pure);
    expectSameMetadata(first.reflected, second.reflected);
}
