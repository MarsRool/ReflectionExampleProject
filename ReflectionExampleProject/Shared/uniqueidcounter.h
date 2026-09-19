#pragma once

template <typename Outer, auto id>
struct UniqueIdCounter
{
    // TODO: maybe make friend func isDefined inline
    struct Generator
    {
        friend constexpr auto isDefined(UniqueIdCounter)
        { return true; }
    };
    friend constexpr auto isDefined(UniqueIdCounter);

    template <typename Tag = UniqueIdCounter, auto = isDefined(Tag{})>
    static constexpr auto exists(std::size_t)
    { return true; }

    static constexpr auto exists(...)
    { return Generator(), false; }
};

template <typename Outer = int, std::size_t id = 0, typename Tag>
constexpr auto uniqueId(Tag tag)
{
    if constexpr (!UniqueIdCounter<Outer, id>::exists(id))
        return id;
    else
        return uniqueId<Outer, id + 1, Tag>(tag);
}
