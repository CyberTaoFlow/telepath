
/*
 *
 *
 * Encoding/decoding to/from base64
 * taken from google :)
 *
 */

#ifndef _base64_h
#define _base64_h

#include <string.h>

unsigned char * base64_code (unsigned char * S, int SIZE = 0);
unsigned char * base64_decode2 (unsigned char * S, int SIZE = 0);
unsigned char *base64_decode(const char *data, size_t input_length, size_t & output_length);

#endif
