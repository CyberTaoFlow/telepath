#ifndef _ICARUS_CONFIG_H_
#define _ICARUS_CONFIG_H_

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <limits.h>
#include <string>
#include <sys/time.h>
#include <arpa/inet.h>

//using namespace std;

#define ENABLE_SEQ_TO_SKIP
#define HAVE_STRCASESTR 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_GETUID
#define SIZEOF_VOID_P 4

#ifndef EAFNOSUPPORT
# define EAFNOSUPPORT EINVAL
#endif

#define NS_INADDRSZ 4
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ 2

typedef long long bro_int_t;
typedef unsigned long long bro_uint_t;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef unsigned long long uint64;

//#define DEBUG_MSG fprintf(stderr

namespace BifEnum { namespace Tunnel { enum Type {
	NONE,
	IP,
	AYIYA,
	TEREDO,
	SOCKS,
}; } }


struct data_chunk_t {
	int length;
	const char* data;
};

#ifndef HAVE_IPPROTO_IPV4
#define IPPROTO_IPV4 4
#endif

//#define min(a,b) (((a)<(b))?(a):(b))

#endif
