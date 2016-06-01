#ifndef conn_h
#define conn_h

#include "ipaddr.h"

struct ConnID {
	IPAddr src_addr;
	IPAddr dst_addr;
	uint32 src_port;
	uint32 dst_port;
	bool is_one_way;	// if true, don't canonicalize order
};


static inline int addr_port_canon_lt(const IPAddr& addr1, uint32 p1,
					const IPAddr& addr2, uint32 p2)
	{
	return addr1 < addr2 || (addr1 == addr2 && p1 < p2);
	}



#endif