#include <stdio.h>
#include <pcap.h>

#include <map>
#include <string>
#include "config.h"
//#include <netinet/ip.h>

#include "ip.h"
#include "ipaddr.h"
#include "frag.h"
//#include "dict.h"
#include "tunnelencapsulation.h"
#include "conn.h"
#include "tcp.h"

#ifndef WIN32
#include <netinet/in.h>
//#include <netinet/ip.h>
#include <netinet/udp.h>
#endif

double network_time;
unsigned int global_time_sec = 0;
unsigned int global_time_usec = 0;

// debug number of GET and POST requests
bool net_raw_debug = true;
unsigned long long raw_get = 0;
unsigned long long raw_post = 0;
unsigned long long good_get = 0;
unsigned long long good_post = 0;

typedef std::pair<IPAddr, IPAddr> IPPair;
typedef std::pair<EncapsulatingConn, double> TunnelActivity;
typedef std::map<IPPair, TunnelActivity> IPTunnelMap;
IPTunnelMap ip_tunnels;

void DoNextPacket(double t, const struct pcap_pkthdr* hdr,
				const IP_Hdr* ip_hdr, const u_char* const pkt,
				int hdr_size, const EncapsulationStack* encapsulation);
void DoNextInnerPacket(double t, const struct pcap_pkthdr* hdr,
		const IP_Hdr* inner, const EncapsulationStack* prev,
		const EncapsulatingConn& ec);
void dispatcher_handler(u_char *, const struct pcap_pkthdr *, const u_char *);
void Weird(const char* name, const struct pcap_pkthdr* hdr,
                        const u_char* pkt, const EncapsulationStack* encap = 0);
void Weird(const char* name, const IP_Hdr* ip, const EncapsulationStack* encap);
FragReassembler* NextFragment(double t, const IP_Hdr* ip, const u_char* pkt);
bool CheckHeaderTrunc(int proto, uint32 len, uint32 caplen, const struct pcap_pkthdr* h,
	const u_char* p, const EncapsulationStack* encap);
int ParseIPPacket(int caplen, const u_char* const pkt, int proto, IP_Hdr*& inner);
TCP_Analyzer * HandleTcpPacket(const IP_Hdr* ip, const u_char* pkt_data, int len, int caplen);
//void Remove(Connection* c);
void Remove(FragReassembler* f);
void RemoveTcpSession(TCP_Analyzer * tcp);
void Drain();
void clean_bro_memory();
//extern void loghex(const unsigned char * data, size_t size);

namespace BifConst { namespace Tunnel {  size_t max_depth = 10;  }  }
namespace BifConst { namespace Tunnel {  int enable_ip = 0;  }  }
namespace BifConst { namespace Tunnel {  bool enable_ayiya = 1;  }  }
namespace BifConst { namespace Tunnel {  bool enable_teredo = 1;  }  }

/*
BifConst::Tunnel::enable_ip = 1;
BifConst::Tunnel::enable_ip = 1;
BifConst::Tunnel::enable_teredo = 1;
*/

const int ignore_checksums = 0;
//const unsigned int port_80 = ntohs(80);
//static const unsigned int port_443 = ntohs(443);
#define LINE_LEN 16

static int datalink = 0;
static unsigned int headersize = 0;

#include <map>
#include <list>
//#ifdef WIN32
//#include <hash_map>
//#else
//#include <ext/hash_map>
//#endif
#include <algorithm>

#ifdef WIN32
stdext::hash_map< std::string, FragReassembler * > fragments;
stdext::hash_map< std::string, TCP_Analyzer * > tcp_connections;
#else
//__gnu_cxx::hash_map< std::string, FragReassembler * > fragments;
//__gnu_cxx::hash_map< std::string, TCP_Analyzer * > tcp_connections;
//std::unordered_map< std::string, FragReassembler * > fragments;
//std::unordered_map< std::string, TCP_Analyzer * > tcp_connections;
std::map< std::string, FragReassembler * > fragments;
std::map< std::string, TCP_Analyzer * > tcp_connections;

#endif

std::list<FragReassembler * > fragments_expiration_queue;
std::list<TCP_Analyzer * > tcp_connections_expiration_queue;
std::list<TCP_Analyzer * > tcp_connections_expiration_queue_last_ack;
std::vector<TCP_Analyzer * > tcp_connections_expired;

#ifdef WIN32
bool check_port(unsigned int sport, unsigned int dport, bool & source_is_server)
{
	static unsigned int port_80 = ntohs(80);
	static unsigned int port_443 = ntohs(443);
	if (dport == port_80 || dport == port_443)
	{
			source_is_server = false;
			return true;
	} else if (sport == port_80 || sport == port_443)
	{
			source_is_server = true;
			return true;
	}
	return false;
}
#else
extern bool check_port(unsigned int sport, unsigned int dport, bool & source_is_server);
#endif


/*
	PDict(Connection) tcp_conns;
	PDict(Connection) udp_conns;
	PDict(Connection) icmp_conns;
	PDict(FragReassembler) fragments;
*/

int get_link_header_size(int dl)
	{
	switch ( dl ) {
	case DLT_NULL:
		return 4;

	case DLT_EN10MB:
		return 14;

	case DLT_FDDI:
		return 13 + 8;	// fddi_header + LLC

#ifdef DLT_LINUX_SLL
	case DLT_LINUX_SLL:
		return 16;
#endif

	case DLT_PPP_SERIAL:	// PPP_SERIAL
		return 4;

	case DLT_RAW:
		return 0;
	}

	return -1;
}


static pcap_t * pcap_fp;
int open_pcap_pipe(FILE *pipe)
{
	tcp_connections_expired.reserve(10000);
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_fp = pcap_fopen_offline(pipe, errbuf);
	//pcap_fp = pcap_open_offline("-", errbuf);
	if (pcap_fp == NULL)
	{
		fprintf(stderr, "Error open pipe\n");
		return -1;	
	}
        datalink = pcap_datalink(pcap_fp);
        headersize = get_link_header_size(datalink);
        printf("data link: %d\n", datalink);
        printf("header size: %d\n", headersize);

        /* read and dispatch packets until EOF is reached */
        //pcap_loop(pcap_fp, 0, dispatcher_handler, NULL);

        //pcap_close(fp);

	return 0;
}
int read_pcap_pipe()
{
	//printf("read from pcap pipe\n");
	//pcap_loop(fp, 0, dispatcher_handler, NULL);
	return pcap_dispatch(pcap_fp, 10, dispatcher_handler, NULL);
}

int close_pcap_pipe()
{
	if (pcap_fp!=NULL){
		pcap_close(pcap_fp);
		pcap_fp=NULL;	
	}
	return 1;
}

int readfile(const char * fname)
{
	float lost_get = 0;
	float lost_post = 0;
	tcp_connections_expired.reserve(10000);

	pcap_t *fp;
	char errbuf[PCAP_ERRBUF_SIZE];
	
	/*
	if(argc != 2)
	{	
		printf("usage: %s filename", argv[0]);
		return -1;

	}
	*/
	/// argv[1] 
	/* Open the capture file */
	fp = pcap_open_offline(fname, errbuf);
	if (fp == NULL)
	{
		fprintf(stderr,"\nUnable to open the file.%s\n", errbuf);
		return -1;
	}

	datalink = pcap_datalink(fp);
	headersize = get_link_header_size(datalink);
	printf("data link: %d\n", datalink);
	printf("header size: %d\n", headersize);

	/* read and dispatch packets until EOF is reached */
	pcap_loop(fp, 0, dispatcher_handler, NULL);
	clean_bro_memory();
	pcap_close(fp);
	if (raw_get)
		lost_get = (((float)(raw_get-good_get) * 100) /  raw_get);
	if (raw_post)
		lost_post = (((float)(raw_post-good_post) * 100) / raw_post);
	printf("net stats: raw get(%lld), good get(%lld), lost(%lld - %f%%), raw post(%lld), good post(%lld), lost(%lld - %f%%)\n",
				raw_get, good_get, (raw_get-good_get), lost_get,
				raw_post, good_post, (raw_post-good_post), lost_post);

	return 0;
}



void dispatcher_handler(u_char *temp1, 
						const struct pcap_pkthdr *header, 
						const u_char *pkt_data)
{
	//u_int i=0;
	
	/*
	 * unused variable
	 */
	//(VOID*)temp1;
	const unsigned char * data = pkt_data;

	unsigned int pkt_hdr_size = headersize;

	// Unfortunately some packets on the link might have MPLS labels
	// while others don't. That means we need to ask the link-layer if
	// labels are in place.
	bool have_mpls = false;

	int protocol = 0;

	DEBUG_MSG("---------------------------------------\n");

	switch ( datalink ) {
	case DLT_NULL:
		{
		protocol = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];

		// From the Wireshark Wiki: "AF_INET6, unfortunately, has
		// different values in {NetBSD,OpenBSD,BSD/OS},
		// {FreeBSD,DragonFlyBSD}, and {Darwin/Mac OS X}, so an IPv6
		// packet might have a link-layer header with 24, 28, or 30
		// as the AF_ value." As we may be reading traces captured on
		// platforms other than what we're running on, we accept them
		// all here.
		if ( protocol != AF_INET
		     && protocol != AF_INET6
		     && protocol != 24
		     && protocol != 28
		     && protocol != 30 )
			{
				DEBUG_MSG("non_ip_packet_in_null_transport\n");
				return;
			}

		break;
		}

	case DLT_EN10MB:
		{
		// Get protocol being carried from the ethernet frame.
		protocol = (data[12] << 8) + data[13];

		// MPLS carried over the ethernet frame.
		if ( protocol == 0x8847 )
		{
			have_mpls = true;
		}
		// VLAN carried over ethernet frame.
		else if ( protocol == 0x8100 )
		{
			//DEBUG_MSG("in vlan !\n");
			data += get_link_header_size(datalink);
			data += 4; // Skip the vlan header
			pkt_hdr_size = 0;
		}
		break;
		}

	case DLT_PPP_SERIAL:
		{
		// Get PPP protocol.
		protocol = (data[2] << 8) + data[3];

		if ( protocol == 0x0281 )
		{
			// MPLS Unicast
			have_mpls = true;
		} else if ( protocol != 0x0021 && protocol != 0x0057 )
		{
			// Neither IPv4 nor IPv6.
			DEBUG_MSG("non_ip_packet_in_ppp_encapsulation\n");
			data = 0;
			return;
		}
		break;
		}
	}

	if ( have_mpls )
	{
		DEBUG_MSG("have_mpls\n");
		// Remove the data link layer
		data += get_link_header_size(datalink);

		// Denote a header size of zero before the IP header
		pkt_hdr_size = 0;

		// Skip the MPLS label stack.
		bool end_of_stack = false;

		while ( ! end_of_stack )
		{
			end_of_stack = *(data + 2) & 0x01;
			data += 4;
		}
	}

	/* print pkt timestamp and pkt len */
	//DEBUG_MSG("%ld:%ld (%ld)\n", header->ts.tv_sec, header->ts.tv_usec, header->len);
	network_time = (double)header->ts.tv_sec;
	//DEBUG_MSG("net time1: %f\n", network_time);
	network_time += (double)header->ts.tv_usec/1000000.0 ;
	//DEBUG_MSG("net time2: %f\n", network_time);
	global_time_sec  = header->ts.tv_sec;
	global_time_usec = header->ts.tv_usec;

	/*
	// Print the packet
	for (i=1; (i < header->caplen + 1 ) ; i++)
	{
		printf("%.2x ", pkt_data[i-1]);
		if ( (i % LINE_LEN) == 0) printf("\n");
	}
	printf("\n");
	// Print the packet
	for (i=1; (i < header->caplen + 1 ) ; i++)
	{
		if (isprint(pkt_data[i-1]))
			printf("%c", pkt_data[i-1]);
		else
			printf(".");				
		if ( (i % 64) == 0) printf("\n");
	}
	*/

//	pkt_hdr_size -=4;
	
	const struct ip* ip_hdr = 0;
	//const unsigned char * ip_data = 0;
	if ( header->caplen < pkt_hdr_size + sizeof(struct ip) )
	{
		DEBUG_MSG("packet small 1\n");
		return;
	}

	//ip_hdr = reinterpret_cast<const struct ip*>(pkt_data + pkt_hdr_size);
	ip_hdr = reinterpret_cast<const struct ip*>(data + pkt_hdr_size);
	if ( header->caplen < unsigned(pkt_hdr_size + (ip_hdr->ip_hl << 2)) )
	{
		DEBUG_MSG("packet small 2\n");
		return;
	}

	/*
	printf("\n---------------------------------------\n");
	printf("src: %d.%d.%d.%d , dest: %d.%d.%d.%d\n",
		ip_hdr->ip_src & 0xff, (ip_hdr->ip_src >> 8) & 0xff, (ip_hdr->ip_src >> 16) & 0xff,(ip_hdr->ip_src >> 24) & 0xff,
		ip_hdr->ip_dst & 0xff, (ip_hdr->ip_dst >> 8) & 0xff, (ip_hdr->ip_dst >> 16) & 0xff,(ip_hdr->ip_dst >> 24) & 0xff);
	*/
	//printf("get ip_data\n");
	//ip_data = data + (ip_hdr->ip_hl << 2);
	
	unsigned int caplen = header->caplen - pkt_hdr_size;

	if ( ip_hdr->ip_v == 4 )
	{
		//printf("going to DoNextPacket\n");
		IP_Hdr ip_hdr2( ip_hdr, false);
		DoNextPacket(network_time, header, &ip_hdr2, data, pkt_hdr_size, 0);
	}
	else if ( ip_hdr->ip_v == 6 )
	{
		DEBUG_MSG("we have ipv6\n");	
		if ( caplen < sizeof(struct ip6_hdr) )
		{
			DEBUG_MSG("truncated_IP\n");
			return;
		}
		IP_Hdr ip_hdr2((const struct ip6_hdr*) ip_hdr, false, caplen);
		DoNextPacket(network_time, header, &ip_hdr2, data, pkt_hdr_size, 0);
	} else {
		DEBUG_MSG("unknown ip format %d\n", ip_hdr->ip_v);
	}
	return;
}

void DoNextPacket(double t, const struct pcap_pkthdr* hdr,
				const IP_Hdr* ip_hdr, const u_char* const pkt,
				int hdr_size, const EncapsulationStack* encapsulation)
	{
	uint32 caplen = hdr->caplen - hdr_size;
	const struct ip* ip4 = ip_hdr->IP4_Hdr();

	uint32 len = ip_hdr->TotalLen();
	if ( hdr->len < len + hdr_size )
		{
		Weird("truncated_IP", hdr, pkt, encapsulation);
		return;
		}

	int ip_hdr_len = ip_hdr->HdrLen();
	if ( ! ignore_checksums && ip4 &&
	     ones_complement_checksum((void*) ip4, ip_hdr_len, 0) != 0xffff )
		{
		Weird("bad_IP_checksum", hdr, pkt, encapsulation);
		return;
		}

	FragReassembler* f = 0;

	if ( ip_hdr->IsFragment() )
		{
		//dump_this_packet = 1;	// always record fragments

		if ( caplen < len )
			{
			Weird("incompletely_captured_fragment", ip_hdr, encapsulation);

			// Don't try to reassemble, that's doomed.
			// Discard all except the first fragment (which
			// is useful in analyzing header-only traces)
			//if ( ip_hdr->FragOffset() != 0 )
				return;
			}
		else
			{
			f = NextFragment(t, ip_hdr, pkt + hdr_size);
			const IP_Hdr* ih = f->ReassembledPkt();
			if ( ! ih )
				// It didn't reassemble into anything yet.
				return;

			ip4 = ih->IP4_Hdr();
			ip_hdr = ih;

			caplen = len = ip_hdr->TotalLen();
			ip_hdr_len = ip_hdr->HdrLen();
			}
		}

	static time_t start = time(NULL);
	static time_t last_drain = start;
	time_t now = time(NULL);
	if (now > last_drain+2)
	{
		// free old bufers
		Drain();
		last_drain = now;
	}
	if (now > start + 30)
	{
		float lost_get = 0;
		float lost_post = 0;
		if (raw_get)
			lost_get = (((float)(raw_get-good_get) * 100) /  raw_get);
		if (raw_post)
			lost_post = (((float)(raw_post-good_post) * 100) / raw_post);
		printf("net stats: # ip fragmentation sessions (" SIZET_FMT "), tcp sessions(" SIZET_FMT "), tcp expiration sessions(" SIZET_FMT "), tcp sessions to reuse(" SIZET_FMT ")\n",
			fragments_expiration_queue.size(),
			tcp_connections_expiration_queue.size(),
			tcp_connections_expiration_queue_last_ack.size(),
			tcp_connections_expired.size());
		printf("net stats: raw get(%lld), good get(%lld), lost(%lld - %f%%), raw post(%lld), good post(%lld), lost(%lld - %f%%)\n",
				raw_get, good_get, (raw_get-good_get), lost_get,
				raw_post, good_post, (raw_post-good_post), lost_post);
		start = time(NULL);
	}

	len -= ip_hdr_len;	// remove IP header
	caplen -= ip_hdr_len;

	// We stop building the chain when seeing IPPROTO_ESP so if it's
	// there, it's always the last.
	if ( ip_hdr->LastHeader() == IPPROTO_ESP )
		{
		Remove(f);
		// Can't do more since upper-layer payloads are going to be encrypted.
		return;
		}

#ifdef ENABLE_MOBILE_IPV6
	// We stop building the chain when seeing IPPROTO_MOBILITY so it's always
	// last if present.
	if ( ip_hdr->LastHeader() == IPPROTO_MOBILITY )
		{
		dump_this_packet = 1;

		if ( ! ignore_checksums && mobility_header_checksum(ip_hdr) != 0xffff )
			{
			Weird("bad_MH_checksum", hdr, pkt, encapsulation);
			Remove(f);
			return;
			}

		if ( mobile_ipv6_message )
			{
			val_list* vl = new val_list();
			vl->append(ip_hdr->BuildPktHdrVal());
			mgr.QueueEvent(mobile_ipv6_message, vl);
			}

		if ( ip_hdr->NextProto() != IPPROTO_NONE )
			Weird("mobility_piggyback", hdr, pkt, encapsulation);

		Remove(f);
		return;
		}
#endif

	int proto = ip_hdr->NextProto();
	
	if (proto == IPPROTO_UDP && (proto != IPPROTO_TCP && proto != IPPROTO_ICMP) )
	{
		// This is not a protocol we understand.
		Remove(f);
		return;
	}

	if ( CheckHeaderTrunc(proto, len, caplen, hdr, pkt, encapsulation) )
		{
		Remove(f);
		return;
		}

	const u_char* data = ip_hdr->Payload();

	switch ( proto ) {
	case IPPROTO_TCP:
		{
		//const struct tcphdr* tp = (const struct tcphdr *) data;
		//unsigned int src_port = tp->th_sport;
		//unsigned int dst_port = tp->th_dport;
		//id.is_one_way = 0;
		//if (src_port != port_80 && src_port != port_443 && dst_port != port_80 && dst_port != port_443)
		//{
		//	Remove(f);
		//	return;
		//}


#ifdef BRO_DEBUG
		const struct tcphdr* tp = (const struct tcphdr *) data;
		unsigned int src_port = tp->th_sport;
		unsigned int dst_port = tp->th_dport;
		unsigned int tcp_len = ip_hdr->PayloadLen() - tp->th_off * 4;
		unsigned int seq = ntohl(tp->th_seq);
		unsigned int ack = ntohl(tp->th_ack);
		//if (9500 != ntohs(src_port) && 9500 !=  ntohs(dst_port))
		//{
		//	Remove(f);
		//	return;
		//}
		DEBUG_MSG("%s:%d => %s:%d : %s%s%s%s S:0x%08X A:0x%08X NS:0x%08X L:0x%04X(%u)\n",
			ip_hdr->SrcAddr().AsURIString().c_str(), ntohs(src_port), 
			ip_hdr->DstAddr().AsURIString().c_str(), ntohs(dst_port), 
			(tp->th_flags & TH_SYN) ? "S ": "",
			(tp->th_flags & TH_FIN) ? "F ": "",
			(tp->th_flags & TH_RST) ? "R ": "",
			(tp->th_flags & TH_URG) ? "U ": "",
			seq, ack,
			seq + tcp_len + ((tp->th_flags & TH_SYN)? 1:0),
			tcp_len, tcp_len
			);
		//loghex(data+tp->th_off * 4, min(16*3, tcp_len));
#endif
		
		//TCP_Analyzer * tcp;
		HandleTcpPacket(ip_hdr, data, len, caplen) ;
		Remove(f);
		
		//d = &tcp_conns;
			break;
		}

/*
	case IPPROTO_UDP:
		{
		const struct udphdr* up = (const struct udphdr *) data;
		id.src_port = up->uh_sport;
		id.dst_port = up->uh_dport;
		id.is_one_way = 0;
		d = &udp_conns;
		break;
		}
*/

/*
	case IPPROTO_ICMP:
		{
		const struct icmp* icmpp = (const struct icmp *) data;

		id.src_port = icmpp->icmp_type;
		id.dst_port = ICMP4_counterpart(icmpp->icmp_type,
						icmpp->icmp_code,
						id.is_one_way);

		id.src_port = htons(id.src_port);
		id.dst_port = htons(id.dst_port);

		d = &icmp_conns;
		break;
		}

	case IPPROTO_ICMPV6:
		{
		const struct icmp* icmpp = (const struct icmp *) data;

		id.src_port = icmpp->icmp_type;
		id.dst_port = ICMP6_counterpart(icmpp->icmp_type,
						icmpp->icmp_code,
						id.is_one_way);

		id.src_port = htons(id.src_port);
		id.dst_port = htons(id.dst_port);

		d = &icmp_conns;
		break;
		}
*/
	case IPPROTO_IPV4:
	case IPPROTO_IPV6:
		{
		if ( ! BifConst::Tunnel::enable_ip )
			{
			Weird("IP_tunnel", ip_hdr, encapsulation);
			Remove(f);
			return;
			}

		if ( encapsulation &&
		     encapsulation->Depth() >= BifConst::Tunnel::max_depth )
			{
			Weird("exceeded_tunnel_max_depth", ip_hdr, encapsulation);
			Remove(f);
			return;
			}

		// Check for a valid inner packet first.
		IP_Hdr* inner = 0;
		int result = ParseIPPacket(caplen, data, proto, inner);

		if ( result < 0 )
			Weird("truncated_inner_IP", ip_hdr, encapsulation);

		else if ( result > 0 )
			Weird("inner_IP_payload_length_mismatch", ip_hdr, encapsulation);

		if ( result != 0 )
			{
			delete inner;
			Remove(f);
			return;
			}

		// Look up to see if we've already seen this IP tunnel, identified
		// by the pair of IP addresses, so that we can always associate the
		// same UID with it.
		IPPair tunnel_idx;
		if ( ip_hdr->SrcAddr() < ip_hdr->DstAddr() )
			tunnel_idx = IPPair(ip_hdr->SrcAddr(), ip_hdr->DstAddr());
		else
			tunnel_idx = IPPair(ip_hdr->DstAddr(), ip_hdr->SrcAddr());

		IPTunnelMap::iterator it = ip_tunnels.find(tunnel_idx);

		if ( it == ip_tunnels.end() )
			{
			EncapsulatingConn ec(ip_hdr->SrcAddr(), ip_hdr->DstAddr());
			ip_tunnels[tunnel_idx] = TunnelActivity(ec, network_time);
			//timer_mgr->Add(new IPTunnelTimer(network_time, tunnel_idx));
			}
		else
			it->second.second = network_time; //network_time;

		DoNextInnerPacket(t, hdr, inner, encapsulation,
		                  ip_tunnels[tunnel_idx].first);

		Remove(f);
		return;
		}

	case IPPROTO_NONE:
		{
		// If the packet is encapsulated in Teredo, then it was a bubble and
		// the Teredo analyzer may have raised an event for that, else we're
		// not sure the reason for the No Next header in the packet.
		if ( ! ( encapsulation &&
		     encapsulation->LastType() == BifEnum::Tunnel::TEREDO ) )
			Weird("ipv6_no_next", hdr, pkt);

		Remove(f);
		return;
		}

	default:
		//Weird(fmt("unknown_protocol_%d", proto), hdr, pkt, encapsulation);
		Remove(f);
		return;
	}
	}

void DoNextInnerPacket(double t, const struct pcap_pkthdr* hdr,
		const IP_Hdr* inner, const EncapsulationStack* prev,
		const EncapsulatingConn& ec)
	{
	struct pcap_pkthdr fake_hdr;
	fake_hdr.caplen = fake_hdr.len = inner->TotalLen();

	if ( hdr )
		fake_hdr.ts = hdr->ts;
	else
		{
		fake_hdr.ts.tv_sec = global_time_sec;
		fake_hdr.ts.tv_usec = global_time_usec;;
//		    ((network_time - (double)fake_hdr.ts.tv_sec) * 1000000);
//		fake_hdr.ts.tv_usec = (suseconds_t)
//		    ((network_time - (double)fake_hdr.ts.tv_sec) * 1000000);
		}

	const u_char* pkt = 0;

	if ( inner->IP4_Hdr() )
		pkt = (const u_char*) inner->IP4_Hdr();
	else
		pkt = (const u_char*) inner->IP6_Hdr();

	EncapsulationStack* outer = prev ?
			new EncapsulationStack(*prev) : new EncapsulationStack();
	outer->Add(ec);

	DoNextPacket(t, &fake_hdr, inner, pkt, 0, outer);

	delete inner;
	delete outer;
	}


bool CheckHeaderTrunc(int proto, uint32 len, uint32 caplen,
                                   const struct pcap_pkthdr* h,
                                   const u_char* p, const EncapsulationStack* encap)
	{
	uint32 min_hdr_len = 0;
	switch ( proto ) {
	case IPPROTO_TCP:
		min_hdr_len = sizeof(struct tcphdr);
		break;
	case IPPROTO_UDP:
		min_hdr_len = sizeof(struct udphdr);
		break;
	case IPPROTO_IPV4:
		min_hdr_len = sizeof(struct ip);
		break;
	case IPPROTO_IPV6:
		min_hdr_len = sizeof(struct ip6_hdr);
		break;
	case IPPROTO_NONE:
		min_hdr_len = 0;
		break;
	case IPPROTO_ICMP:
	case IPPROTO_ICMPV6:
	default:
		// Use for all other packets.
		min_hdr_len = ICMP_MINLEN;
		break;
	}

	if ( len < min_hdr_len )
		{
		Weird("truncated_header", h, p, encap);
		return true;
		}

	if ( caplen < min_hdr_len )
		{
		Weird("internally_truncated_header", h, p, encap);
		return true;
		}

	return false;
	}

int ParseIPPacket(int caplen, const u_char* const pkt, int proto,
		IP_Hdr*& inner)
	{
	if ( proto == IPPROTO_IPV6 )
		{
		if ( caplen < (int)sizeof(struct ip6_hdr) )
			return -1;

		inner = new IP_Hdr((const struct ip6_hdr*) pkt, false, caplen);
		}

	else if ( proto == IPPROTO_IPV4 )
		{
		if ( caplen < (int)sizeof(struct ip) )
			return -1;

		inner = new IP_Hdr((const struct ip*) pkt, false);
		}

	else
		reporter->InternalError("Bad IP protocol version in DoNextInnerPacket");

	if ( (uint32)caplen != inner->TotalLen() )
		return (uint32)caplen < inner->TotalLen() ? -1 : 1;

	return 0;
	}

void Weird(const char* name, const struct pcap_pkthdr* hdr,
                        const u_char* pkt, const EncapsulationStack* encap)
	{
	//if ( hdr )
	//	dump_this_packet = 1;

	if ( encap && encap->LastType() != BifEnum::Tunnel::NONE )
		reporter->Weird(fmt("%s_in_tunnel", name));
	else
		reporter->Weird(name);
	}

void Weird(const char* name, const IP_Hdr* ip,
                        const EncapsulationStack* encap)
	{
	if ( encap && encap->LastType() != BifEnum::Tunnel::NONE )
		reporter->Weird(ip->SrcAddr(), ip->DstAddr(),
		                fmt("%s_in_tunnel", name));
	else
		reporter->Weird(ip->SrcAddr(), ip->DstAddr(), name);
	}

FragReassembler* NextFragment(double t, const IP_Hdr* ip, const u_char* pkt)
{
	FragReassembler* f = NULL;
	uint32 frag_id = ip->ID();

	std::string key = ip->SrcAddr().AsString();
	key.append(ip->DstAddr().AsString());
	int_to_string(frag_id, key);

	FragmentsIterator f_itr = fragments.find(key);
	if (f_itr == fragments.end())
	{
		f = new FragReassembler(/*this,*/ ip, pkt, /*h, */ t);
		std::pair<FragmentsIterator, bool> p1 = fragments.insert( std::pair<std::string, FragReassembler*>(key, f));
		f->SetContainerIterator( p1.first );
		//fragments.Insert(h, f);
	} else {
		// remove from expiration queue
		f = f_itr->second;
		FragmentsExpirationIterator & frag_itr = f->GetExpirationQueueIterator();
		fragments_expiration_queue.erase( frag_itr );
		// add new fragment 
		f->AddFragment(t, ip, pkt);
	}
	// check if we can reassemble this packet
	if (f->ReassembledPkt() != NULL)
	{
		// this fragment was reassembled
		// we can remove if from fragments hash
		if (f_itr != fragments.end())
			fragments.erase(f_itr);

	} else {
		// set to end of expiration queue
		fragments_expiration_queue.push_back( f );
		f->SetExpirationQueueIterator ( --fragments_expiration_queue.end() );
	}
	return f;
	}

TCP_Analyzer * HandleTcpPacket(const IP_Hdr* ip, const u_char* pkt_data, int len, int caplen)
{
	const struct tcphdr* tp = (const struct tcphdr *) pkt_data;
	TCP_Analyzer * tcp = NULL;
	static std::string key;
	unsigned int src_port = tp->th_sport;
	unsigned int dst_port = tp->th_dport;

	//BuildConnKey(ip->SrcAddr(), src_port, ip->DstAddr(), dst_port, key);
	//printf("key: %s\n", key.c_str());
	ip->BuildKey(key);

	TCPConnectionsIterator tcp_itr = tcp_connections.find(key);

	if (tcp_itr == tcp_connections.end())
	{
		bool source_is_server;
		if (!check_port( src_port, dst_port, source_is_server))
		{
			DEBUG_MSG("No suitable ports found!\n");
			return NULL;
		}
		if (tcp_connections_expired.size())
		{
			tcp = tcp_connections_expired.back();
			tcp_connections_expired.pop_back();

			// new tcp connection !
			if ( source_is_server == false )
			{
				tcp->Reset( ip->SrcAddr(), src_port, ip->DstAddr(), dst_port );
			} else {
				tcp->Reset( ip->DstAddr(), dst_port, ip->SrcAddr(), src_port );
			}

		} else {
			// new tcp connection !
			if ( source_is_server == false )
			{
				tcp = new TCP_Analyzer( ip->SrcAddr(), src_port, ip->DstAddr(), dst_port );
			} else {
				tcp = new TCP_Analyzer( ip->DstAddr(), dst_port, ip->SrcAddr(), src_port );
			}
		}
		if (tcp->IsFullyClosed())
		{
			//delete tcp;
			RemoveTcpSession(tcp);
			return NULL;
		}
		std::pair< TCPConnectionsIterator, bool> p1 = tcp_connections.insert( std::pair<std::string, TCP_Analyzer*>(key, tcp));
		tcp->SetContainerIterator ( p1.first );
	} else {
		// old connection
		tcp = tcp_itr->second;
		if (tcp->IsFullyClosed())
		{
			//TCPConnectionsExpirationIterator & frag_itr = tcp->GetExpirationQueueIterator();
			//tcp_connections_expiration_queue_last_ack.erase( frag_itr );
			DEBUG_MSG("send more data on closed socket, probably last ACK !\n");
			TCPConnectionsExpirationIterator & frag_itr = tcp->GetExpirationQueueIterator();
			tcp_connections_expiration_queue_last_ack.erase( frag_itr );
			if (tp->th_flags & TH_SYN && tp->th_ack == 0)
			{
				tcp->Done();
				// we got a SYN packet -> a new tcp connection !
				bool source_is_server;
				check_port( src_port, dst_port, source_is_server);
				if ( source_is_server == false )
				{
					tcp->Reset( ip->SrcAddr(), src_port, ip->DstAddr(), dst_port );
				} else {
					tcp->Reset( ip->DstAddr(), dst_port, ip->SrcAddr(), src_port );
				}
				tcp->DeliverPacket(len, pkt_data, ip, caplen);
				tcp_connections_expiration_queue.push_back( tcp );
				tcp->SetExpirationQueueIterator ( --tcp_connections_expiration_queue.end() );
			} else {
				tcp->DeliverPacket(len, pkt_data, ip, caplen);
				// relocate to the end of the waiting queue !
				tcp_connections_expiration_queue_last_ack.push_back( tcp );
				tcp->SetExpirationQueueIterator ( --tcp_connections_expiration_queue_last_ack.end() );
			}
			return NULL;
		} else {
			TCPConnectionsExpirationIterator & frag_itr = tcp->GetExpirationQueueIterator();
			tcp_connections_expiration_queue.erase( frag_itr );
		}
	}

	tcp->DeliverPacket(len, pkt_data, ip, caplen);
	if (tcp->IsFullyClosed())
	{
		tcp_connections_expiration_queue_last_ack.push_back( tcp );
		tcp->SetExpirationQueueIterator ( --tcp_connections_expiration_queue_last_ack.end() );
		return NULL;
	}
	tcp_connections_expiration_queue.push_back( tcp );
	tcp->SetExpirationQueueIterator ( --tcp_connections_expiration_queue.end() );
	return NULL;
}
/*
void Remove(Connection* c)
	{
	HashKey* k = c->Key();
	if ( k )
		{
		c->CancelTimers();

		TCP_Analyzer* ta = (TCP_Analyzer*) c->GetRootAnalyzer();
		if ( ta && c->ConnTransport() == TRANSPORT_TCP )
			{
			assert(ta->GetTag() == AnalyzerTag::TCP);
			TCP_Endpoint* to = ta->Orig();
			TCP_Endpoint* tr = ta->Resp();

			tcp_stats.StateLeft(to->state, tr->state);
			}

		//if ( c->IsPersistent() )
		//	persistence_serializer->Unregister(c);

		c->Done();

		if ( connection_state_remove )
			c->Event(connection_state_remove, 0);

		// Zero out c's copy of the key, so that if c has been Ref()'d
		// up, we know on a future call to Remove() that it's no
		// longer in the dictionary.
		c->ClearKey();

		switch ( c->ConnTransport() ) {
		case TRANSPORT_TCP:
			if ( ! tcp_conns.RemoveEntry(k) )
				reporter->InternalError("connection missing");
			break;

		case TRANSPORT_UDP:
			if ( ! udp_conns.RemoveEntry(k) )
				reporter->InternalError("connection missing");
			break;

		case TRANSPORT_ICMP:
			if ( ! icmp_conns.RemoveEntry(k) )
				reporter->InternalError("connection missing");
			break;

		case TRANSPORT_UNKNOWN:
			reporter->InternalError("unknown transport when removing connection");
			break;
		}

		Unref(c);
		delete k;
		}
	}
*/

void Remove(FragReassembler* f)
	{
	if ( ! f ) return;
	delete f;
	return;
	}

void RemoveTcpSession(TCP_Analyzer * tcp)
{
	if (tcp_connections_expired.size() < 10000)
	{
		tcp->Done();
		tcp_connections_expired.push_back(tcp);
	}
	else
	{
		delete tcp;
	}
}

void Drain()
	{
	FragReassembler* f;
	TCP_Analyzer * tcp;

	if (!fragments_expiration_queue.empty())
		{
		FragmentsExpirationIterator frag_itr = fragments_expiration_queue.begin();
		while ( frag_itr != fragments_expiration_queue.end())
			{
			f = (*frag_itr);
			if (!f->IsExpired(network_time))
				break;
			DEBUG_MSG("Remove expired ip fragment!\n");
			FragmentsIterator & f_itr = f->GetContainerIterator();
			fragments.erase(f_itr);
			fragments_expiration_queue.erase( frag_itr++ );
			delete f;
			}
		}
	if (!tcp_connections_expiration_queue.empty())
		{
		TCPConnectionsExpirationIterator tcp_exp_itr = tcp_connections_expiration_queue.begin();
		while ( tcp_exp_itr != tcp_connections_expiration_queue.end())
			{
			tcp = (*tcp_exp_itr);
			if (!tcp->IsExpired(network_time))
				break;
			DEBUG_MSG("Remove expired tcp connection! time: %f\n", network_time);
			tcp->Dump();
			TCPConnectionsIterator & tcp_itr = tcp->GetContainerIterator();
			tcp_connections.erase(tcp_itr);
			tcp_connections_expiration_queue.erase( tcp_exp_itr++ );
			//delete tcp;
			RemoveTcpSession(tcp);
			}

		}
	if (!tcp_connections_expiration_queue_last_ack.empty())
		{
		TCPConnectionsExpirationIterator tcp_exp_itr = tcp_connections_expiration_queue_last_ack.begin();
		while ( tcp_exp_itr != tcp_connections_expiration_queue_last_ack.end())
			{
			tcp = (*tcp_exp_itr);
			if (!tcp->IsExpired(network_time))
				break;
			DEBUG_MSG("Remove expired tcp connection2! time: %f\n", network_time);
			tcp->Dump();
			TCPConnectionsIterator & tcp_itr = tcp->GetContainerIterator();
			tcp_connections.erase(tcp_itr);
			tcp_connections_expiration_queue_last_ack.erase( tcp_exp_itr++ );
			//delete tcp;
			RemoveTcpSession(tcp);
			}

		}
	}

void clean_bro_memory()
	{
	FragReassembler* f;

	if (!fragments_expiration_queue.empty())
		{
		FragmentsExpirationIterator frag_itr = fragments_expiration_queue.begin();
		while ( frag_itr != fragments_expiration_queue.end())
			{
			f = (*frag_itr);
			FragmentsIterator & f_itr = f->GetContainerIterator();
			fragments.erase(f_itr);
			fragments_expiration_queue.erase( frag_itr++ );
			delete f;
			}
		}
	if (!tcp_connections_expired.empty())
	{
		std::vector<TCP_Analyzer * >::iterator tcp_itr2 = tcp_connections_expired.begin();
		while ( tcp_itr2 != tcp_connections_expired.end() )
		{
			delete *tcp_itr2;
			tcp_itr2++;
		}
		tcp_connections_expired.clear();
	}
	/*
	TCPConnectionsIterator tcp_itr = tcp_connections.begin();
	while ( tcp_itr != tcp_connections.end() )
	{
		delete tcp_itr->second;
		tcp_itr++;
	}
	tcp_connections.clear();
	tcp_connections_expiration_queue.clear();
	tcp_connections_expiration_queue_last_ack.clear();
	return;
	*/

	TCP_Analyzer * tcp;
	if (!tcp_connections_expiration_queue.empty())
		{
		TCPConnectionsExpirationIterator tcp_exp_itr = tcp_connections_expiration_queue.begin();
		while ( tcp_exp_itr != tcp_connections_expiration_queue.end())
			{
			tcp = (*tcp_exp_itr);
			TCPConnectionsIterator & tcp_itr = tcp->GetContainerIterator();
			tcp_connections.erase(tcp_itr);
			tcp_connections_expiration_queue.erase( tcp_exp_itr++ );
			delete tcp;
			}

		}
	if (!tcp_connections_expiration_queue_last_ack.empty())
		{
		TCPConnectionsExpirationIterator tcp_exp_itr = tcp_connections_expiration_queue_last_ack.begin();
		while ( tcp_exp_itr != tcp_connections_expiration_queue_last_ack.end())
			{
			tcp = (*tcp_exp_itr);
			TCPConnectionsIterator & tcp_itr = tcp->GetContainerIterator();
			tcp_connections.erase(tcp_itr);
			tcp_connections_expiration_queue_last_ack.erase( tcp_exp_itr++ );
			delete tcp;
			}

		}
	if (tcp_connections.size() > 0)
		{
		printf("We still have object that were not deleted in tcp connection map !\n");
		}
	}
