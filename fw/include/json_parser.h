#ifndef AWARE_JSON_PARSER_H
#define AWARE_JSON_PARSER_H

#include "mbed.h"
#include "substring.h"

extern "C"
{
#include "jsmn.h"
}

namespace aware
{

class JsonObject;

class JsonParser
{
  friend class JsonObject;

public:
  JsonParser();
  ~JsonParser();

  // Parse a JSON string
  JsonObject parseJsonString(const char *json_string);

  // Parse a JSON file
  JsonObject parseJsonFile(const char *file_path);

private:
  int _r;
  jsmntok_t _t[MBED_CONF_APP_CONFIG_MAX_TOKENS];
  const char *_json_string;
  char *_internal_json_string;
  jsmn_parser _p;
};

class JsonObject
{

public:
  JsonObject(JsonParser *parser, int offset, int length);
  ~JsonObject();

  // Test if there is more tokens
  bool has_next() const;

  // Matchers, test if the next elemnt matches a specific element type
  bool match_string() const;
  bool match_boolean() const;
  bool match_null() const;
  bool match_number() const;
  inline bool match_int() const { return match_number(); }
  inline bool match_double() const { return match_number(); }
  bool match_object() const;
  bool match_array() const;

  // Consumers, consume the next element assuming it's of the specific type
  Substring consume_string();
  int consume_int();
  double consume_double();
  bool consume_boolean();
  void consume_null();
  JsonObject consume_object();
  inline JsonObject consume_array() { return consume_object(); }

  void consume_next();

private:
  inline const char *json_string()
  {
    return _p->_json_string;
  }

  bool match_chars(const char *chars, const char ch) const;
  bool match_true() const;
  bool match_false() const;
  bool match_primitive() const;

  void consume_one();
  int end_of_object();

  int _r;
  int _i;
  int _offset;
  JsonParser *_p;
};

} // namespace aware

#endif