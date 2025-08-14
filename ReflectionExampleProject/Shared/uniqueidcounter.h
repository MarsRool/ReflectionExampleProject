#pragma once
#include <cstdint>

template <typename Outer, auto id>
struct UniqueIdCounter
{
    struct Generator
    {
        friend consteval auto isDefined(UniqueIdCounter)
        { return true; }
    };
    friend consteval auto isDefined(UniqueIdCounter);

    template <typename Tag = UniqueIdCounter, auto = isDefined(Tag{})>
    static consteval auto exists(auto)
    { return true; }

    static consteval auto exists(...)
    { return Generator(), false; }
};

template <typename Outer = int, std::size_t id = 0, typename = decltype([]{})>
consteval auto uniqueId()
{
    if constexpr (!UniqueIdCounter<Outer, id>::exists(id))
        return id;
    else
        return uniqueId<Outer, id + 1>();
}
