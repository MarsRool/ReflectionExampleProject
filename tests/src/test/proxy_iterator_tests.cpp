#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "extensions/comparisonextension.h"
#include "extensions/tojsonextension.h"
#include "extensions/tostringextension.h"

namespace proxy_iterator_tests
{

struct IntProxy
{
    const int* value;

    operator int() const noexcept
    {
        return *value;
    }
};

struct ProxyIterator
{
    using iterator_category = std::input_iterator_tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = IntProxy;

    const int* current = nullptr;

    IntProxy operator*() const noexcept
    {
        return { current };
    }

    ProxyIterator& operator++() noexcept
    {
        ++current;
        return *this;
    }

    ProxyIterator operator++(int) noexcept
    {
        const auto previous = *this;
        ++*this;
        return previous;
    }

    bool operator==(const ProxyIterator& other) const noexcept
    {
        return current == other.current;
    }

    bool operator!=(const ProxyIterator& other) const noexcept
    {
        return !(*this == other);
    }
};

struct ProxyRange
{
    std::array<int, 3> values;

    ProxyIterator begin() const noexcept
    {
        return { values.data() };
    }

    ProxyIterator end() const noexcept
    {
        return { values.data() + values.size() };
    }
};

static_assert(std::is_same_v<
    decltype(extensions::impl::dereferenceValue(std::declval<ProxyIterator&>())),
    int>);

static_assert(std::is_same_v<
    decltype(extensions::impl::dereferenceValue(
        std::declval<std::vector<int>::const_iterator&>())),
    const int&>);

static_assert(std::is_same_v<
    decltype(extensions::impl::dereferenceValue(
        std::declval<std::vector<std::unique_ptr<int>>::const_iterator&>())),
    const std::unique_ptr<int>&>);

TEST(ProxyIterators, ProxyDereferenceAndExtensions)
{
    const ProxyRange left{ { 1, 2, 3 } };
    const ProxyRange equal{ { 1, 2, 3 } };
    const ProxyRange larger{ { 1, 2, 4 } };

    auto it = left.begin();
    EXPECT_EQ(extensions::impl::dereferenceValue(it), 1);

    using Checker = reflection::TrueTypeChecker;
    EXPECT_EQ(extensions::impl::compareValue<Checker>(left, equal), 0);
    EXPECT_LT(extensions::impl::compareValue<Checker>(left, larger), 0);
    EXPECT_GT(extensions::impl::compareValue<Checker>(larger, left), 0);

    nlohmann::json output;
    ASSERT_TRUE(extensions::impl::valueToJson<Checker>(left, output));
    EXPECT_EQ(output, nlohmann::json::array({ 1, 2, 3 }));
    EXPECT_EQ(extensions::impl::valueToString<Checker>(left), "[ 1, 2, 3 ]");
}

TEST(ProxyIterators, OrdinaryDereferencePreservesReferences)
{
    const std::vector<int> values{ 4, 5, 6 };
    auto it = values.begin();

    EXPECT_EQ(std::addressof(extensions::impl::dereferenceValue(it)),
              std::addressof(*it));
}

} // namespace proxy_iterator_tests
