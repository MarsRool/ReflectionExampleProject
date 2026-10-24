#include <map>
#include <string>

#include "extensions/tojsonextension.h"

struct Record
{
    std::map<std::string, int> dictionary;
};

REFLECTION_NONINTRUSIVE(Record, &Record::dictionary)

void convert()
{
    Record record;
    nlohmann::json output;
    extensions::convertToJson(record, output);
}
