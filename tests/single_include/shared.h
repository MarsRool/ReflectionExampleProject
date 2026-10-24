#pragma once

#include <reflection/reflection.h>

namespace single_include_smoke
{

struct IntrusiveBase
{
    REFLECTION_BEGIN(IntrusiveBase)
    REFLECTION_PROPERTY(int, id, 5)
    REFLECTION_END()
};

struct IntrusiveRecord : IntrusiveBase
{
    REFLECTION_BEGIN(IntrusiveRecord)
    REFLECTION_BASE_CLASS(IntrusiveBase)
    REFLECTION_PROPERTY(int, score, 7)
    REFLECTION_PROPERTY_CUSTOM(int, custom, "custom_value", 11)
    REFLECTION_END()
};

struct NonIntrusiveBase
{
    int flag = 13;
};
REFLECTION_NONINTRUSIVE(NonIntrusiveBase, &NonIntrusiveBase::flag)

struct NonIntrusiveRecord : NonIntrusiveBase
{
    int value = 17;
};
REFLECTION_NONINTRUSIVE(NonIntrusiveRecord, &NonIntrusiveRecord::value)
REFLECTION_BASE_CLASSES_NONINTRUSIVE(NonIntrusiveRecord, NonIntrusiveBase)

template <typename T>
struct GenericRecord
{
    T value{};
};
REFLECTION_TEMPLATE_NONINTRUSIVE(
    (typename T), (T), GenericRecord, &GenericRecord<T>::value)

inline constexpr char scoreKey[] = "score";
inline constexpr char customKey[] = "custom_value";
inline constexpr char valueKey[] = "value";
inline constexpr char baseKey[] = "_base_IntrusiveBase";

const void* propertiesAddressFromSecondTu();
int scoreFromSecondTu();

} // namespace single_include_smoke
