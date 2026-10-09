#pragma once
#include <array>
#include <string>
#include <vector>
#include <memory>
#include "Reflection/reflection.h"

namespace pure
{

struct BaseTestObject
{
    std::string name{};
};

struct NestedTestObject : public BaseTestObject
{
    bool isValid{};
};

template <typename T>
struct TemplateNestedTestObject : public BaseTestObject
{
    T value{};
};

struct AnotherBaseTestObject
{
    std::string anotherName{};
};

struct TestObject : public BaseTestObject, public AnotherBaseTestObject
{
    using DoubleArray = std::array<double, 3>;

    // Note, these two fields not participate in reflection
    // they are used as a storage for pointer properties
    std::shared_ptr<NestedTestObject> nestedStorage =
        std::make_shared<NestedTestObject>();
    std::shared_ptr<TemplateNestedTestObject<float>> templateNestedStorage =
        std::make_shared<TemplateNestedTestObject<float>>();

    std::size_t age{};
    NestedTestObject nested{};
    TemplateNestedTestObject<int> templateNested1{};
    TemplateNestedTestObject<std::string> templateNested2{};
    NestedTestObject* nestedPtr{ nestedStorage.get() };
    TemplateNestedTestObject<float>* templateNestedPtr{ templateNestedStorage.get() };
    std::vector<std::string> stringArr{};
    DoubleArray realArr{};
};


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

} // namespace pure
