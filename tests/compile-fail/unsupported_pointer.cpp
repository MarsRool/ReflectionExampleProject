#include "extensions/tojsonextension.h"

struct Record
{
    int* pointer = nullptr;
};

REFLECTION_NONINTRUSIVE(Record, &Record::pointer)

void convert()
{
    Record record;
    nlohmann::json output;
    extensions::convertToJson(record, output);
}
