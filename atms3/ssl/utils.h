#ifndef SSL_UTILS_HTTP
#define SSL_UTILS_HTTP

#include <stdio.h>

void loghex(const unsigned char * data, size_t size);

//#define SSL_DEBUG

#ifndef SSL_DEBUG
#define DEBUG_SSL_MSG(...)
#define DEBUG_SSL_HEX(...)
#else
#define DEBUG_SSL_MSG(x,...)        fprintf(stdout, x, ##__VA_ARGS__)
#define DEBUG_SSL_HEX(pos,size)          loghex(pos,size)
#endif

#ifndef SIZET_FMT
#ifdef __WIN32__ // or whatever
#define SIZET_FMT "%lu"
#define SSIZET_FMT "%ld"
#else
#define SIZET_FMT "%zu"
#define SSIZET_FMT "%zd"
#endif
#endif

#endif
