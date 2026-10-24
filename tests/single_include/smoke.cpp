#include "shared.h"

#include <array>
#include <cstddef>
#include <string_view>

namespace single_include_smoke
{

using IntrusiveMeta = reflection::StaticPropertyMeta<IntrusiveRecord>;
using NonIntrusiveMeta = reflection::StaticPropertyMeta<NonIntrusiveRecord>;
using GenericMeta = reflection::StaticPropertyMeta<GenericRecord<int>>;

static_assert(IntrusiveMeta::size() == 3);
static_assert(IntrusiveMeta::contains<scoreKey>());
static_assert(IntrusiveMeta::contains<customKey>());
static_assert(IntrusiveMeta::contains<baseKey>());
static_assert(NonIntrusiveMeta::size() == 2);
static_assert(GenericMeta::size() == 1);

int run()
{
    IntrusiveRecord intrusive{};
    if (intrusive.id != 5 || intrusive.score != 7 || intrusive.custom != 11)
        return 1;

    const auto score = IntrusiveMeta::at<scoreKey>();
    score->set(intrusive, 42);
    if (score->get(intrusive) != 42)
        return 2;

    const auto custom = IntrusiveMeta::at<customKey>();
    if (custom->get(intrusive) != 11)
        return 3;

    const NonIntrusiveRecord nonIntrusive{};
    if (NonIntrusiveMeta::at<valueKey>()->get(nonIntrusive) != 17)
        return 4;

    std::array<std::string_view, 3> nonIntrusiveNames{};
    std::size_t nonIntrusiveVisited = 0;
    reflection::StaticPropertyTraversal::forEach(nonIntrusive,
        [&](auto entry, const auto&)
    {
        if (nonIntrusiveVisited < nonIntrusiveNames.size())
            nonIntrusiveNames[nonIntrusiveVisited] = decltype(entry)::key;
        ++nonIntrusiveVisited;
    });
    if (nonIntrusiveVisited != nonIntrusiveNames.size() ||
        nonIntrusiveNames != std::array<std::string_view, 3>{
            "_base_NonIntrusiveBase", "flag", "value" })
        return 9;

    GenericRecord<int> generic{};
    GenericMeta::at<valueKey>()->set(generic, 23);
    if (generic.value != 23)
        return 5;

    std::array<std::string_view, 4> names{};
    std::size_t visited = 0;
    reflection::StaticPropertyTraversal::forEach(intrusive,
        [&](auto entry, const auto&)
    {
        if (visited < names.size())
            names[visited] = decltype(entry)::key;
        ++visited;
    });

    const std::array<std::string_view, 4> expected
    {
        "_base_IntrusiveBase", "id", "score", "custom_value"
    };
    if (visited != names.size() || names != expected)
        return 6;

    if (&IntrusiveMeta::properties != propertiesAddressFromSecondTu())
        return 7;

    if (scoreFromSecondTu() != 7)
        return 8;

    return 0;
}

} // namespace single_include_smoke

int main()
{
    return single_include_smoke::run();
}
