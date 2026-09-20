#pragma once

template <typename Outer, auto id>
struct UniqueIdCounter
{
    struct Generator
    {
        friend constexpr auto isDefined(UniqueIdCounter)
        { return true; }
    };

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

    friend constexpr auto isDefined(UniqueIdCounter);

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

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
