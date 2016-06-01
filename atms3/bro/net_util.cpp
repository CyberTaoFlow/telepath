// See the file "COPYING" in the main distribution directory for copyright.

//#include "config.h"

#include <sys/types.h>
//#include <sys/socket.h>

//#include <netinet/in.h>

//#include <arpa/inet.h>

//#include "reporter.h"
#include "net_util.h"
//#include "ipaddr.h"
#include "ip.h"

// - adapted from tcpdump
// Returns the ones-complement checksum of a chunk of b short-aligned bytes.
int ones_complement_checksum(const void* p, int b, uint32 sum)
	{
	const u_short* sp = (u_short*) p;	// better be aligned!

	b /= 2;	// convert to count of short's

	/* No need for endian conversions. */
	while ( --b >= 0 )
		sum += *sp++;

	while ( sum > 0xffff )
		sum = (sum & 0xffff) + (sum >> 16);

	return sum;
	}

int ones_complement_checksum(const IPAddr& a, uint32 sum)
	{
	const uint32* bytes;
	int len = a.GetBytes(&bytes);
	return ones_complement_checksum(bytes, len*4, sum);
	}

int icmp_checksum(const struct icmp* icmpp, int len)
	{
	uint32 sum;

	if ( len % 2 == 1 )
		// Add in pad byte.
		sum = htons(((const u_char*) icmpp)[len - 1] << 8);
	else
		sum = 0;

	sum = ones_complement_checksum((void*) icmpp, len, sum);

	return sum;
	}

#ifdef ENABLE_MOBILE_IPV6
int mobility_header_checksum(const IP_Hdr* ip)
	{
	const ip6_mobility* mh = ip->MobilityHeader();

	if ( ! mh ) return 0;

	uint32 sum = 0;
	uint8 mh_len = 8 + 8 * mh->ip6mob_len;

	if ( mh_len % 2 == 1 )
		reporter->Weird(ip->SrcAddr(), ip->DstAddr(), "odd_mobility_hdr_len");

	sum = ones_complement_checksum(ip->SrcAddr(), sum);
	sum = ones_complement_checksum(ip->DstAddr(), sum);
	// Note, for IPv6, strictly speaking the protocol and length fields are
	// 32 bits rather than 16 bits.  But because the upper bits are all zero,
	// we get the same checksum either way.
	sum += htons(IPPROTO_MOBILITY);
	sum += htons(mh_len);
	sum = ones_complement_checksum(mh, mh_len, sum);

	return sum;
	}
#endif

int icmp6_checksum(const struct icmp* icmpp, const IP_Hdr* ip, int len)
	{
	// ICMP6 uses the same checksum function as ICMP4 but a different
	// pseudo-header over which it is computed.
	uint32 sum;

	if ( len % 2 == 1 )
		// Add in pad byte.
		sum = htons(((const u_char*) icmpp)[len - 1] << 8);
	else
		sum = 0;

	// Pseudo-header as for UDP over IPv6 above.
	sum = ones_complement_checksum(ip->SrcAddr(), sum);
	sum = ones_complement_checksum(ip->DstAddr(), sum);
	uint32 l = htonl(len);
	sum = ones_complement_checksum((void*) &l, 4, sum);

	uint32 addl_pseudo = htons(IPPROTO_ICMPV6);
	sum = ones_complement_checksum((void*) &addl_pseudo, 4, sum);

	sum = ones_complement_checksum((void*) icmpp, len, sum);

	return sum;
	}


#define CLASS_A 0x00000000
#define CLASS_B 0x80000000
#define CLASS_C 0xc0000000
#define CLASS_D 0xe0000000
#define CLASS_E 0xf0000000

#define CHECK_CLASS(addr,class) (((addr) & (class)) == (class))
char addr_to_class(uint32 addr)
	{
	if ( CHECK_CLASS(addr, CLASS_E) )
		return 'E';
	else if ( CHECK_CLASS(addr, CLASS_D) )
		return 'D';
	else if ( CHECK_CLASS(addr, CLASS_C) )
		return 'C';
	else if ( CHECK_CLASS(addr, CLASS_B) )
		return 'B';
	else
		return 'A';
	}

/**
 * Convert data[0..3] to 32 bit integer
 */
uint32 extract_uint32(const u_char* data)
	{
	uint32 val;

	val = data[0] << 24;
	val |= data[1] << 16;
	val |= data[2] << 8;
	val |= data[3];

	return val;
	}
