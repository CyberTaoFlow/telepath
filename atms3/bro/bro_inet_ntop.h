#ifndef BRO_INET_NTOP_H
#define BRO_INET_NTOP_H

#include "config.h"
/*
#ifdef __cplusplus
extern "C" {
#endif
*/

//#include <sys/socket.h>

const char * 
bro_inet_ntop(int af, const void * __restrict src, char * __restrict dst,
    socklen_t size);
/*
#ifdef __cplusplus
}
#endif
*/

#endif
