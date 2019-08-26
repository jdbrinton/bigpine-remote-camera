#ifndef BASE64_H
#define BASE64_H

#include <stdint.h>

void base64_encode(char* out, const char *data, uint32_t input_length);

#endif
