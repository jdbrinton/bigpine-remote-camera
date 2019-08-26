#include <stdlib.h>
#include <algorithm>
#include "json_parser.h"

namespace aware
{

/************************************************************************************************************
 * JsonParser
 ***********************************************************************************************************/

JsonParser::JsonParser() : _r(0),  _json_string(NULL), _internal_json_string(NULL)
{
    jsmn_init(&_p);
}

JsonParser::~JsonParser()
{
    if (_internal_json_string != NULL)
    {
        free(_internal_json_string);
    }
}

JsonObject JsonParser::parseJsonFile(const char *file_path)
{
    FILE *file = fopen(file_path, "r");
    if (file != NULL)
    {
        // Initialize buffer
        fseek(file, 0, SEEK_END);
        long len = ftell(file);
        fseek(file, 0, SEEK_SET);
        _internal_json_string = (char *)malloc(len + 1);

        // Read configuration file into the buffer
        fread(_internal_json_string, len, 1, file);

        // Add null terminator to the end
        _internal_json_string[len] = 0;
    }
    return parseJsonString(_internal_json_string);
}

JsonObject JsonParser::parseJsonString(const char *json_string)
{
    // Note, json_string must stay in memory until parser has finished
    _json_string = json_string;

    if (_json_string != NULL)
    {
        // Parse JSON String
        _r = jsmn_parse(&_p, _json_string, strlen(_json_string), _t, sizeof(_t) / sizeof(_t[0]));
        return JsonObject(this, 1, _r);
    }
    else
    {
        return JsonObject(this, 0, 0);
    }
}

/************************************************************************************************************
 * JsonObject
 ***********************************************************************************************************/

JsonObject::JsonObject(JsonParser *parser, int start, int end) : _r(end), _i(start), _p(parser) {}

JsonObject::~JsonObject() {}

bool JsonObject::match_chars(const char *chars, const char ch) const
{
    if (chars == NULL)
    {
        return false;
    }

    const char *other = chars;

    while (*other != 0)
    {
        if (*other == ch)
        {
            return true;
        }
        other++;
    }
    return false;
}

bool JsonObject::match_string() const
{
    const jsmntok_t *tok = &(_p->_t[_i]);
    return tok->type == JSMN_STRING;
}

bool JsonObject::match_true() const
{
    const jsmntok_t *tok = &(_p->_t[_i]);
    Substring s(_p->_json_string, tok->start, 4);
    return s == "true";
}

bool JsonObject::match_false() const
{
    const jsmntok_t *tok = &(_p->_t[_i]);
    Substring s(_p->_json_string, tok->start, 4);
    return s == "false";
}

bool JsonObject::match_null() const
{
    if (match_primitive())
    {
        const jsmntok_t *tok = &(_p->_t[_i]);
        Substring s(_p->_json_string, tok->start, 4);
        return s == "null";
    }
    else
    {
        return false;
    }
}

bool JsonObject::match_boolean() const
{
    return (match_primitive() && (match_true() || match_false()));
}

bool JsonObject::match_number() const
{
    if (match_primitive())
    {
        const jsmntok_t *tok = &(_p->_t[_i]);
        const char * str = _p->_json_string + tok->start;
        const char * end = _p->_json_string + tok->end;
        if (str < end && *str == '-')
        {
            ++str;
        }
        if (str < end && *str == '0')
        {
            ++str;
        }
        else
        {
            if (str < end && match_chars("123456789", *str))
            {
                ++str;
                while (str < end && match_chars("0123456789", *str))
                {
                    ++str;
                }
            }
            else
            {
                return false;
            }
        }
        if (str < end && *str == '.')
        {
            ++str;
            while (str < end && match_chars("0123456789", *str))
            {
                ++str;
            }
        }
        if (str < end && ((*str == 'e') || (*str == 'E')))
        {
            ++str;
            if (str < end && ((*str == '+') || (*str == '-')))
            {
                ++str;
            }
            if (str < end && match_chars("0123456789", *str))
            {
                ++str;
                while (str < end && match_chars("0123456789", *str))
                {
                    ++str;
                }
            }
            else
            {
                return false;
            }
        }
        if (str == end)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool JsonObject::match_primitive() const
{
    const jsmntok_t *tok = &(_p->_t[_i]);
    return tok->type == JSMN_PRIMITIVE;
}

bool JsonObject::match_object() const
{
    const jsmntok_t *tok = &(_p->_t[_i]);
    return tok->type == JSMN_OBJECT;
}

bool JsonObject::match_array() const
{
    const jsmntok_t *tok = &(_p->_t[_i]);
    return tok->type == JSMN_ARRAY;
}

int JsonObject::consume_int()
{
    const jsmntok_t *tok = &(_p->_t[_i]);
    int num = atoi(_p->_json_string + tok->start);
    consume_next();
    return num;
}

double JsonObject::consume_double()
{
    const jsmntok_t *tok = &(_p->_t[_i]);
    double num = atof(_p->_json_string + tok->start);
    consume_next();
    return num;
}

Substring JsonObject::consume_string()
{
    const jsmntok_t *tok = &(_p->_t[_i]);
    Substring str(_p->_json_string, tok->start, tok->end - tok->start);
    consume_next();
    return str;
}

bool JsonObject::consume_boolean()
{
    const jsmntok_t *tok = &(_p->_t[_i]);
    Substring str(_p->_json_string, tok->start, tok->end - tok->start);
    consume_next();
    return str == "true";
}

void JsonObject::consume_null()
{
    consume_next();
}

int JsonObject::end_of_object()
{
    int i = _i;
    const jsmntok_t *tok = &(_p->_t[i]);
    int current_end = tok->end;
    do {
        tok = &(_p->_t[++i]);
    } while (i < _r && tok->start < current_end);
    return i;
}

JsonObject JsonObject::consume_object()
{
    int r = end_of_object();
    JsonObject obj(_p, _i+1, r);
    _i = r;
    return obj;
}

void JsonObject::consume_one()
{
    ++_i;
}

void JsonObject::consume_next() 
{
    _i = end_of_object();
}


bool JsonObject::has_next() const
{
    return _i < _r;
}



} // namespace aware