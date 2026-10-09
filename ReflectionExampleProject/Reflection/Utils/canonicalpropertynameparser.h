#pragma once
#include <type_traits>
#include <array>
#include "Reflection/Utils/canonicalstaticstring.h"

namespace reflection
{

namespace impl
{

template <typename T, const T* propertyNames>
struct CanonicalPropertyNameParserImpl
{
    static_assert(propertyNames != nullptr);

    struct StaticPropertyNameRange
    {
        std::size_t begin;
        std::size_t end;
    };

    static constexpr bool isCharIdentifier(T c)
    {
        return
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '_';
    }

    static constexpr bool isCharDelimiter(T c)
    {
        return c == ',';
    }

    static constexpr std::size_t getNamesCount()
    {
        if constexpr (propertyNames == nullptr
            || propertyNames[0] == static_cast<T>('\0'))
        {
            return 0;
        }
        else
        {
            std::size_t count = 1;
            std::size_t templateDepth = 0;
            bool containsIdentifier = false;

            for (std::size_t index = 0; propertyNames[index]; ++index)
            {
                const auto c = propertyNames[index];

                if (c == '<')
                {
                    ++templateDepth;
                }
                else if (c == '>')
                {
                    if (templateDepth == 0)
                        return 0;

                    --templateDepth;
                }
                else if (templateDepth == 0)
                {
                    if (isCharDelimiter(c))
                    {
                        if (!containsIdentifier)
                            return 0;

                        ++count;
                        containsIdentifier = false;
                    }
                    else if (isCharIdentifier(c))
                    {
                        containsIdentifier = true;
                    }
                }
            }

            if (!(templateDepth == 0 && containsIdentifier))
            {
                return 0;
            }

            return count;
        }
    }

    static constexpr std::size_t count = getNamesCount();
    static_assert(count != 0, "CanonicalPropertyNameParser: unexpected empty propertyNames");

    using NameRangesArray = std::array<StaticPropertyNameRange, count>;

    static constexpr NameRangesArray getNameRangesArray()
    {
        NameRangesArray array{};

        std::size_t rangeIndex = 0;
        std::size_t index = 0;
        std::size_t templateDepth = 0;

        std::size_t argumentBegin = 0;
        std::size_t argumentEnd = 0;

        bool shouldResetArgument = false;

        while (propertyNames[index])
        {
            const auto c = propertyNames[index];

            if (c == '<')
            {
                ++templateDepth;
                shouldResetArgument = true;
            }
            else if (c == '>')
            {
                --templateDepth;
                shouldResetArgument = true;
            }
            else if (templateDepth == 0)
            {
                if (isCharDelimiter(c))
                {
                    if (!(argumentBegin < argumentEnd && rangeIndex < count))
                        return {};

                    array.at(rangeIndex) = { argumentBegin, argumentEnd };

                    ++rangeIndex;
                    shouldResetArgument = true;
                }
                else if (isCharIdentifier(c))
                {
                    if (shouldResetArgument || argumentBegin == argumentEnd)
                    {
                        argumentBegin = index;
                        argumentEnd = index + 1;
                        shouldResetArgument = false;
                    }
                    else
                    {
                        ++argumentEnd;
                    }
                }
                else
                {
                    shouldResetArgument = true;
                }
            }

            ++index;
        }

        if (!(argumentBegin < argumentEnd && rangeIndex < count))
            return {};

        array.at(rangeIndex) = { argumentBegin, argumentEnd };

        return array;
    }

    static constexpr NameRangesArray nameRangesArray = getNameRangesArray();
    static_assert(nameRangesArray[0].begin < nameRangesArray[0].end,
                  "CanonicalPropertyNameParser: unexpected empty nameRangesArray");

    template <std::size_t index>
    using CanonicalName = CanonicalStaticStringRange<
        propertyNames,
        nameRangesArray[index].begin,
        nameRangesArray[index].end
            - nameRangesArray[index].begin>;
};

} // namespace impl

template <const auto* propertyNames>
using CanonicalPropertyNameParser = impl::CanonicalPropertyNameParserImpl<
    std::remove_cv_t<std::remove_pointer_t<decltype(propertyNames)>>,
    propertyNames>;

} // namespace reflection
