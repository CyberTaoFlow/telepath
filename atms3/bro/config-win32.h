#ifndef _ICARUS_CONFIG_WIN32_H_
#define _ICARUS_CONFIG_WIN32_H_

#include <stdio.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <direct.h>   // for mkdir
//#include <wincrypt.h>  //for random
#include <string>
#include <hash_map>
#include <windows.h>
//using namespace std;

#define srandom srand
#define random rand
#define getpid _getpid
#define write _write
#define open _open
#define snprintf _snprintf
#define mkdir _mkdir
#define strcasecmp _stricmp

#define ENABLE_SEQ_TO_SKIP

#define SIZEOF_VOID_P 4

#ifndef EAFNOSUPPORT
# define EAFNOSUPPORT EINVAL
#endif

#define NS_INADDRSZ 4
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ 2

//#define DEBUG_MSG fprintf(stderr

typedef unsigned char u_char;
typedef unsigned char uint8;
typedef unsigned char uint8_t;
typedef unsigned short uint16;
typedef unsigned short uint16_t;
typedef unsigned int uint32;
typedef int int32;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64;
typedef unsigned long long uint64_t;
typedef long long int64;
typedef long long int64_t;

typedef long long bro_int_t;
typedef unsigned long long bro_uint_t;

typedef int pid_t;

// Versions of realloc/malloc which abort() on out of memory


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

struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};


#endif
