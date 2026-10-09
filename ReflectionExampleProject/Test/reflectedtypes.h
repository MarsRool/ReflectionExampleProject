#pragma once
#include <array>
#include <string>
#include <vector>
#include <memory>
#include "Reflection/reflection.h"

namespace reflected
{

struct BaseTestObject
{
    DECL_REFLECTION_BODY(BaseTestObject)

    DECL_PROPERTY_DEFAULT(std::string, name)
};

struct NestedTestObject : public BaseTestObject
{
    DECL_REFLECTION_BODY(NestedTestObject)
    DECL_BASE_CLASS(BaseTestObject)

    DECL_PROPERTY_DEFAULT(bool, isValid)
};

template <typename T>
struct TemplateNestedTestObject : public BaseTestObject
{
    DECL_REFLECTION_BODY(TemplateNestedTestObject)
    DECL_BASE_CLASS(BaseTestObject)

    DECL_PROPERTY_DEFAULT(T, value)
};

struct AnotherBaseTestObject
{
    DECL_REFLECTION_BODY(AnotherBaseTestObject)

    DECL_PROPERTY_DEFAULT(std::string, anotherName)
};

struct TestObject : public BaseTestObject, public AnotherBaseTestObject
{
    DECL_REFLECTION_BODY(TestObject)
    DECL_BASE_CLASS(BaseTestObject)
    DECL_BASE_CLASS(AnotherBaseTestObject)

    using DoubleArray = std::array<double, 3>;

    // Note, these two fields not participate in reflection
    // they are used as a storage for pointer properties
    std::shared_ptr<NestedTestObject> nestedStorage =
        std::make_shared<NestedTestObject>();
    std::shared_ptr<TemplateNestedTestObject<float>> templateNestedStorage =
        std::make_shared<TemplateNestedTestObject<float>>();

    DECL_PROPERTY_DEFAULT(std::size_t, age)
    DECL_PROPERTY_DEFAULT(NestedTestObject, nested)
    DECL_PROPERTY_DEFAULT(TemplateNestedTestObject<int>, templateNested1)
    DECL_PROPERTY_DEFAULT(TemplateNestedTestObject<std::string>, templateNested2)
    DECL_PROPERTY_INIT(NestedTestObject*, nestedPtr, nestedStorage.get())
    DECL_PROPERTY_INIT(TemplateNestedTestObject<float>*, templateNestedPtr, templateNestedStorage.get())
    DECL_PROPERTY_DEFAULT(std::vector<std::string>, stringArr)
    DECL_PROPERTY_DEFAULT(DoubleArray, realArr)
};

// Note, non-intrusive macroes are NOT necessary
// it's just a check, that reflection can handle both (intrusive and non-intrusive)
// on the same types without breaking or any side effects

DECL_REFLECTION_NONINTRUSIVE(BaseTestObject,
                             &BaseTestObject::name)

DECL_REFLECTION_NONINTRUSIVE(NestedTestObject,
                             &NestedTestObject::isValid)

DECL_REFLECTION_INHERITANCE_NONINTRUSIVE(NestedTestObject, BaseTestObject)

DECL_REFLECTION_TEMPLATE_NONINTRUSIVE(
    (typename T),
    (T),
    TemplateNestedTestObject,
    &TemplateNestedTestObject<T>::value)

DECL_REFLECTION_INHERITANCE_TEMPLATE_NONINTRUSIVE(
    (typename T),
    (T),
    TemplateNestedTestObject,
    BaseTestObject)

DECL_REFLECTION_NONINTRUSIVE(AnotherBaseTestObject,
    &AnotherBaseTestObject::anotherName)

DECL_REFLECTION_NONINTRUSIVE(TestObject,
    &TestObject::age,
    &TestObject::nested,
    &TestObject::templateNested1,
    &TestObject::templateNested2,
    &TestObject::nestedPtr,
    &TestObject::templateNestedPtr,
    &TestObject::stringArr,
    &TestObject::realArr)

DECL_REFLECTION_INHERITANCE_NONINTRUSIVE(TestObject,
    BaseTestObject,
    AnotherBaseTestObject)

} // namespace reflected
