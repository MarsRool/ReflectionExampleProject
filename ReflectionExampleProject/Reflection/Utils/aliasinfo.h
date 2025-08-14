#pragma once
#include <tuple>
#include <vector>
#include <array>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include "Shared/typetester.h"

namespace reflection
{

template <class AliasType, const char rawAlias[]>
class AliasInfo
{
    static constexpr std::size_t MAX_STRING_ARRAY_LENGTH = 256;
    using StringArray = std::array<char, MAX_STRING_ARRAY_LENGTH>;

    class AliasParser
    {
    public:
        static consteval std::size_t calculateAliasesCount(const std::string_view alias)
        {
            constexpr std::size_t MAX_ITERATIONS = 32;

            std::string_view currentAlias = alias;
            std::size_t count = 0;

            for (std::size_t index = 0; index < MAX_ITERATIONS; ++index)
            {
                auto aliasPair = parseNextAlias(currentAlias);
                currentAlias = std::string_view{ currentAlias.begin() + aliasPair.second, currentAlias.end() };

                if (aliasPair.first.empty() || aliasPair.second == 0)
                {
                    return count;
                }
                else
                {
                    ++count;
                }
            }

            return count;
        }

        static consteval auto breakAlias(const std::string_view alias)
        {
            std::string_view currentAlias = alias;
            std::array<std::string_view, aliasesCount> result{};

            for (std::size_t index = 0; index < aliasesCount; ++index)
            {
                auto aliasPair = parseNextAlias(currentAlias);
                currentAlias = std::string_view{ currentAlias.begin() + aliasPair.second, currentAlias.end() };
                result[index] = aliasPair.first;
            }

            return result;
        }

    private:
        static consteval std::pair<std::string_view, std::size_t> parseNextAlias(const std::string_view typeAlias)
        {
            bool foundAnyValidSymbol = false;
            std::size_t firstValidIndex = 0, lastValidIndex = 0;

            for (std::size_t index = 0; index < typeAlias.size(); ++index)
            {
                switch (typeAlias[index])
                {
                case ' ':
                    [[fallthrough]];
                case '\t':
                    [[fallthrough]];
                case '<':
                    [[fallthrough]];
                case '>':
                    [[fallthrough]];
                case ',':
                {
                    if (foundAnyValidSymbol)
                    {
                        return std::make_pair(std::string_view(typeAlias.begin() + firstValidIndex, typeAlias.begin() + lastValidIndex + 1ull),
                                              lastValidIndex + 1ull);
                    }
                    break;
                }

                default:
                {
                    if (!foundAnyValidSymbol)
                    {
                        foundAnyValidSymbol = true;
                        firstValidIndex = index;
                    }
                    lastValidIndex = index;
                    break;
                }
                }
            }

            if (foundAnyValidSymbol)
            {
                return std::make_pair(std::string_view(typeAlias.begin() + firstValidIndex, typeAlias.begin() + lastValidIndex + 1ull),
                                      lastValidIndex + 1ull);
            }
            else
            {
                return std::make_pair(std::string_view(), typeAlias.size());
            }
        }
    };

    class AliasHelper
    {
    public:
        // template <std::size_t size>
        // static consteval std::array<std::string_view, size + 1> combine(std::string_view alias, const std::array<std::string_view, size> aliases)
        // {
        //     std::array<std::string_view, size + 1> result{};
        //     std::copy_n(aliases.begin(), aliases.size(), result.begin());
        //     result[size] = alias;
        //     return result;
        // }

        template <std::size_t length>
        static consteval auto stringToArray(const std::string_view alias)
        {
            std::array<char, length> result{};

            for (std::size_t index = 0; index < length - 1 && index < alias.size(); ++index)
            {
                result[index] = alias[index];
            }

            return result;
        }

        template <std::size_t length, const std::array<char, length> aliasStringArray>
        static consteval std::string_view arrayToStringView()
        {
            return std::string_view(aliasStringArray.begin(), std::find(aliasStringArray.begin(), aliasStringArray.end(), '\0'));
        }

        template <typename T, std::size_t resultSize, std::size_t... sizes>
        static consteval StringArray concatenate(const std::array<T, sizes>&... arrays)
        {
            std::array<T, resultSize> result{};
            std::size_t index{};

            ((std::copy_n(arrays.begin(), std::min<std::size_t>(std::distance(arrays.begin(), std::find(arrays.begin(), arrays.end(), '\0')), sizes), result.begin() + index),
              index += std::min<std::size_t>(std::distance(arrays.begin(), std::find(arrays.begin(), arrays.end(), '\0')), sizes)), ...);

            return result;
        }

        // template <typename T, std::size_t... sizes>
        // static consteval auto concatenate(const std::array<T, sizes>&... arrays)
        // {
        //     std::array<T, (sizes + ...)> result;
        //     std::size_t index{};

        //     ((std::copy_n(arrays.begin(), sizes, result.begin() + index), index += sizes), ...);

        //     return result;
        // }
    };

public:
    static constexpr std::size_t aliasesCount = AliasParser::calculateAliasesCount(std::string_view(rawAlias));
    static constexpr auto aliases = AliasParser::breakAlias(std::string_view(rawAlias));
    static constexpr std::string_view simpleAlias = aliasesCount > 0 ? aliases[0] : std::string_view{};
    static constexpr bool valid = !aliases.empty() && aliasesCount > 0;

private:
    class TypeParser
    {
    public:
        template <class T>
        static constexpr StringArray toString()
        {
            if constexpr (aliasesCount > 1)
            {
                constexpr auto templateUnwrapper = []<template<class...> class U, class... Args>(const U<Args...>&) constexpr
                {
                    return std::tuple<Args...>{};
                };

                using TemplateAliasArgsTuple = decltype(templateUnwrapper(std::declval<AliasType>()));

                return AliasHelper::template concatenate<char, MAX_STRING_ARRAY_LENGTH>(
                    AliasHelper::template stringToArray<MAX_STRING_ARRAY_LENGTH>(simpleAlias),
                    std::array<char, 2>{ '<' },
                    argsToString<>(TemplateAliasArgsTuple{}),
                    std::array<char, 2>{ '>' });
            }
            return AliasHelper::template stringToArray<MAX_STRING_ARRAY_LENGTH>(simpleAlias);
        }

    private:
        template <typename... Args>
        static constexpr StringArray argsToString(const std::tuple<Args...>)
        {
            return unwrapArgs<Args...>();
        }

        template <class T, typename... Args>
        static constexpr StringArray unwrapArgs()
        {
            if constexpr (sizeof...(Args) == 0)
            {
                return singleTypeToString<T>();
            }
            else
            {
                return AliasHelper::template concatenate<char, MAX_STRING_ARRAY_LENGTH>(singleTypeToString<T>(), unwrapArgs<Args...>());
            }
        }

        template <class T>
        static consteval StringArray singleTypeToString()
        {
            using Type = std::remove_all_extents_t<RemoveAllPointersT<std::remove_reference_t<std::remove_cv_t<T>>>>;

            std::string result;

            // scalar types
            if constexpr (std::is_enum_v<Type>)
            {
                result += "enum";
            }
            else if constexpr (std::is_same_v<Type, bool>)
            {
                result += "bool";
            }
            else if constexpr (std::is_null_pointer_v<Type>)
            {
                result += "std::nullptr_t";
            }
            else if constexpr (std::is_integral_v<Type>)
            {
                if constexpr (std::is_unsigned_v<Type>)
                {
                    result += "unsigned ";
                }
                if constexpr (sizeof(Type) <= sizeof(char))
                {
                    result += "char";
                }
                else if constexpr (sizeof(Type) <= sizeof(short int))
                {
                    result += "short int";
                }
                else if constexpr (sizeof(Type) <= sizeof(int))
                {
                    result += "int";
                }
                else if constexpr (sizeof(Type) <= sizeof(long int))
                {
                    result += "long int";
                }
                else /* if constexpr (sizeof(Type) <= sizeof(long long int)) */
                {
                    result += "long long int";
                }
            }
            else if constexpr (std::is_floating_point_v<Type>)
            {
                if constexpr (sizeof(Type) <= sizeof(float))
                {
                    result += "float";
                }
                else if constexpr (sizeof(Type) <= sizeof(double))
                {
                    result += "double";
                }
                else /* if constexpr (sizeof(Type) <= sizeof(long double)) */
                {
                    result += "long double";
                }
            }
            // string types
            else if constexpr (std::is_same_v<Type, std::string>)
            {
                result += "std::string";
            }
            else if constexpr (std::is_same_v<Type, std::string_view>)
            {
                result += "std::string_view";
            }
            // standard containers
            else if constexpr (IsSpecialization<T, std::pair>::value)
            {
                result += "std::pair";
            }
            else if constexpr (IsSpecialization<T, std::tuple>::value)
            {
                result += "std::tuple";
            }
            else if constexpr (IsSpecialization<T, std::vector>::value)
            {
                result += "std::vector";
            }
            else if constexpr (IsSpecialization<T, std::list>::value)
            {
                result += "std::list";
            }
            else if constexpr (IsSpecializationSized<T, std::array>::value)
            {
                result += "std::array";
            }
            else if constexpr (IsSpecialization<T, std::set>::value)
            {
                result += "std::set";
            }
            else if constexpr (IsSpecialization<T, std::map>::value)
            {
                result += "std::map";
            }
            else if constexpr (IsSpecialization<T, std::unordered_set>::value)
            {
                result += "std::unordered_set";
            }
            else if constexpr (IsSpecialization<T, std::unordered_map>::value)
            {
                result += "std::unordered_map";
            }
            // reflection type
            else if constexpr (requires{ T::Meta::aliasInfo.alias; })
            {
                result += T::Meta::aliasInfo.alias;
            }

            if constexpr (std::is_volatile_v<T>)
            {
                result = "volatile " + result;
            }
            if constexpr (std::is_const_v<T>)
            {
                result = "const " + result;
            }

            for (std::size_t index = 0; index < PointersRankV<T>; ++index)
            {
                result += "*";
            }
            if constexpr (std::is_reference_v<T>)
            {
                result += "&";
            }

            for (std::size_t index = 0; index < std::rank_v<T>; ++index)
            {
                result += "[]";
            }

            return AliasHelper::template stringToArray<MAX_STRING_ARRAY_LENGTH>(std::string_view(result));
        }
    };
    static constexpr StringArray aliasArray = TypeParser::template toString<AliasType>();

public:
    static constexpr std::string_view alias = AliasHelper::template arrayToStringView<MAX_STRING_ARRAY_LENGTH, aliasArray>();
};

} // namespace reflection
