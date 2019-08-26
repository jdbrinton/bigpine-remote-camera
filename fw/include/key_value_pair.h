#ifndef AWARE_KEY_VALUE_PAIR_H
#define AWARE_KEY_VALUE_PAIR_H

#include "mbed.h"

namespace aware
{
class Value
{
  public:
    virtual int serialize_to_json_file(FILE *json_file) const = 0;
};

class StringValue : public Value
{
  public:
    StringValue(const char *const value);
    int serialize_to_json_file(FILE *json_file) const override;

  private:
    const char *const _value;
};

class DoubleValue : public Value
{
  public:
    DoubleValue(double value);
    int serialize_to_json_file(FILE *json_file) const override;

  private:
    double _value;
};

class IntValue : public Value
{
  public:
    IntValue(int value);
    int serialize_to_json_file(FILE *json_file) const override;

  private:
    int _value;
};

class KeyValuePair
{
  public:
    KeyValuePair(const char * key, const Value &value);
    inline const char *key() const { return _key; }
    inline const Value & value() const { return _value; } 
    virtual int serialize_to_json_file(FILE *json_file) const;

  private:
    const char *_key;
    const Value &_value;
};

} // namespace aware

#endif