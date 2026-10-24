#include <initializer_list>

#include "extensions/fromjsonextension.h"

struct Record
{
    std::initializer_list<int> values;
};

REFLECTION_NONINTRUSIVE(Record, &Record::values)

void convert()
{
    Record record;
    extensions::convertFromJson(record, nlohmann::json::object());
}
