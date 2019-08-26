#include "key_value_pair.h"

namespace aware
{

StringValue::StringValue(const char *const value) : _value(value) {}

int StringValue::serialize_to_json_file(FILE *json_file) const
{
    fprintf(json_file, "\"%s\"", _value);
    return 0;
}

DoubleValue::DoubleValue(double value) : _value(value) {}

int DoubleValue::serialize_to_json_file(FILE *json_file) const
{
    fprintf(json_file, "%g", _value);
    return 0;
}

IntValue::IntValue(int value) : _value(value) {}

int IntValue::serialize_to_json_file(FILE *json_file) const
{
    fprintf(json_file, "%d", _value);
    return 0;
}

KeyValuePair::KeyValuePair(const char *key, const Value &value) : _key(key), _value(value) {}

int KeyValuePair::serialize_to_json_file(FILE *json_file) const
{
    StringValue(_key).serialize_to_json_file(json_file);
    fprintf(json_file, " : ");
    _value.serialize_to_json_file(json_file);
    return 0;
}

} // namespace aware
