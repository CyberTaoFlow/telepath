// See the file "COPYING" in the main distribution directory for copyright.

#include "ip.h"
#include "tcp.h"
//#include "type.h"
//#include "val.h"
//#include "var.h"

/*
static RecordType* ip4_hdr_type = 0;
static RecordType* ip6_hdr_type = 0;
static RecordType* ip6_ext_hdr_type = 0;
static RecordType* ip6_option_type = 0;
static RecordType* ip6_hopopts_type = 0;
static RecordType* ip6_dstopts_type = 0;
static RecordType* ip6_routing_type = 0;
static RecordType* ip6_fragment_type = 0;
static RecordType* ip6_ah_type = 0;
static RecordType* ip6_esp_type = 0;
static RecordType* ip6_mob_type = 0;
static RecordType* ip6_mob_msg_type = 0;
static RecordType* ip6_mob_brr_type = 0;
static RecordType* ip6_mob_hoti_type = 0;
static RecordType* ip6_mob_coti_type = 0;
static RecordType* ip6_mob_hot_type = 0;
static RecordType* ip6_mob_cot_type = 0;
static RecordType* ip6_mob_bu_type = 0;
static RecordType* ip6_mob_back_type = 0;
static RecordType* ip6_mob_be_type = 0;

*/

void IP_Hdr::BuildKey(std::string & result) const
{
	const struct tcphdr* tp = (const struct tcphdr *) Payload();
	unsigned int src_port = tp->th_sport;
	unsigned int dst_port = tp->th_dport;
	result.clear();
	if (src_port < dst_port)
	{
		result.append((char*)&tp->th_sport, 2);
		result.append((char*)&tp->th_dport, 2);
		if (ip4)
		{
			result.append( (char*)&ip4->ip_src, 4);
			result.append( (char*)&ip4->ip_dst, 4);
		} else {
			result.append( (char*) &ip6->ip6_src, 16);
			result.append( (char*) &ip6->ip6_dst, 16);
		}
	} else if(src_port > dst_port)
	{
		result.append((char*)&tp->th_dport, 2);
		result.append((char*)&tp->th_sport, 2);
		if (ip4)
		{
			result.append( (char*)&ip4->ip_dst, 4);
			result.append( (char*)&ip4->ip_src, 4);
		} else {
			result.append( (char*) &ip6->ip6_dst, 16);
			result.append( (char*) &ip6->ip6_src, 16);
		}
	} else {
		// same ports
		result.append((char*)&tp->th_sport, 2);
		result.append((char*)&tp->th_sport, 2);
		if (ip4)
		{
			if (memcmp((void*)&ip4->ip_src, (void*)&ip4->ip_dst, 4) < 0)
			{
				result.append( (char*)&ip4->ip_src, 4);
				result.append( (char*)&ip4->ip_dst, 4);
			} else {
				result.append( (char*)&ip4->ip_dst, 4);
				result.append( (char*)&ip4->ip_src, 4);
			}
		} else {
			if (memcmp((void*)&ip6->ip6_src, (void*)&ip6->ip6_dst, 16) < 0)
			{
				result.append( (char*) &ip6->ip6_src, 16);
				result.append( (char*) &ip6->ip6_dst, 16);
			} else {
				result.append( (char*) &ip6->ip6_dst, 16);
				result.append( (char*) &ip6->ip6_src, 16);
			}
		}
	}
}

/*
static VectorVal* BuildOptionsVal(const u_char* data, int len)
	{
	VectorVal* vv = new VectorVal(internal_type("ip6_options")->AsVectorType());

	while ( len > 0 )
		{
		const struct ip6_opt* opt = (const struct ip6_opt*) data;
		RecordVal* rv = new RecordVal(hdrType(ip6_option_type, "ip6_option"));
		rv->Assign(0, new Val(opt->ip6o_type, TYPE_COUNT));

		if ( opt->ip6o_type == 0 )
			{
			// Pad1 option
			rv->Assign(1, new Val(0, TYPE_COUNT));
			rv->Assign(2, new StringVal(""));
			data += sizeof(uint8);
			len -= sizeof(uint8);
			}
		else
			{
			// PadN or other option
			uint16 off = 2 * sizeof(uint8);
			rv->Assign(1, new Val(opt->ip6o_len, TYPE_COUNT));
			rv->Assign(2, new StringVal(
			        new BroString(data + off, opt->ip6o_len, 1)));
			data += opt->ip6o_len + off;
			len -= opt->ip6o_len + off;
			}

		vv->Assign(vv->Size(), rv, 0);
		}

	return vv;
	}
*/
/*
RecordVal* IPv6_Hdr::BuildRecordVal(VectorVal* chain) const
	{
	RecordVal* rv = 0;

	switch ( type ) {
	case IPPROTO_IPV6:
		{
		rv = new RecordVal(hdrType(ip6_hdr_type, "ip6_hdr"));
		const struct ip6_hdr* ip6 = (const struct ip6_hdr*)data;
		rv->Assign(0, new Val((ntohl(ip6->ip6_flow) & 0x0ff00000)>>20, TYPE_COUNT));
		rv->Assign(1, new Val(ntohl(ip6->ip6_flow) & 0x000fffff, TYPE_COUNT));
		rv->Assign(2, new Val(ntohs(ip6->ip6_plen), TYPE_COUNT));
		rv->Assign(3, new Val(ip6->ip6_nxt, TYPE_COUNT));
		rv->Assign(4, new Val(ip6->ip6_hlim, TYPE_COUNT));
		rv->Assign(5, new AddrVal(IPAddr(ip6->ip6_src)));
		rv->Assign(6, new AddrVal(IPAddr(ip6->ip6_dst)));
		if ( ! chain )
			chain = new VectorVal(
			    internal_type("ip6_ext_hdr_chain")->AsVectorType());
		rv->Assign(7, chain);
		}
		break;

	case IPPROTO_HOPOPTS:
		{
		rv = new RecordVal(hdrType(ip6_hopopts_type, "ip6_hopopts"));
		const struct ip6_hbh* hbh = (const struct ip6_hbh*)data;
		rv->Assign(0, new Val(hbh->ip6h_nxt, TYPE_COUNT));
		rv->Assign(1, new Val(hbh->ip6h_len, TYPE_COUNT));
		uint16 off = 2 * sizeof(uint8);
		rv->Assign(2, BuildOptionsVal(data + off, Length() - off));

		}
		break;

	case IPPROTO_DSTOPTS:
		{
		rv = new RecordVal(hdrType(ip6_dstopts_type, "ip6_dstopts"));
		const struct ip6_dest* dst = (const struct ip6_dest*)data;
		rv->Assign(0, new Val(dst->ip6d_nxt, TYPE_COUNT));
		rv->Assign(1, new Val(dst->ip6d_len, TYPE_COUNT));
		uint16 off = 2 * sizeof(uint8);
		rv->Assign(2, BuildOptionsVal(data + off, Length() - off));
		}
		break;

	case IPPROTO_ROUTING:
		{
		rv = new RecordVal(hdrType(ip6_routing_type, "ip6_routing"));
		const struct ip6_rthdr* rt = (const struct ip6_rthdr*)data;
		rv->Assign(0, new Val(rt->ip6r_nxt, TYPE_COUNT));
		rv->Assign(1, new Val(rt->ip6r_len, TYPE_COUNT));
		rv->Assign(2, new Val(rt->ip6r_type, TYPE_COUNT));
		rv->Assign(3, new Val(rt->ip6r_segleft, TYPE_COUNT));
		uint16 off = 4 * sizeof(uint8);
		rv->Assign(4, new StringVal(new BroString(data + off, Length() - off, 1)));
		}
		break;

	case IPPROTO_FRAGMENT:
		{
		rv = new RecordVal(hdrType(ip6_fragment_type, "ip6_fragment"));
		const struct ip6_frag* frag = (const struct ip6_frag*)data;
		rv->Assign(0, new Val(frag->ip6f_nxt, TYPE_COUNT));
		rv->Assign(1, new Val(frag->ip6f_reserved, TYPE_COUNT));
		rv->Assign(2, new Val((ntohs(frag->ip6f_offlg) & 0xfff8)>>3, TYPE_COUNT));
		rv->Assign(3, new Val((ntohs(frag->ip6f_offlg) & 0x0006)>>1, TYPE_COUNT));
		rv->Assign(4, new Val(ntohs(frag->ip6f_offlg) & 0x0001, TYPE_BOOL));
		rv->Assign(5, new Val(ntohl(frag->ip6f_ident), TYPE_COUNT));
		}
		break;

	case IPPROTO_AH:
		{
		rv = new RecordVal(hdrType(ip6_ah_type, "ip6_ah"));
		rv->Assign(0, new Val(((ip6_ext*)data)->ip6e_nxt, TYPE_COUNT));
		rv->Assign(1, new Val(((ip6_ext*)data)->ip6e_len, TYPE_COUNT));
		rv->Assign(2, new Val(ntohs(((uint16*)data)[1]), TYPE_COUNT));
		rv->Assign(3, new Val(ntohl(((uint32*)data)[1]), TYPE_COUNT));
		rv->Assign(4, new Val(ntohl(((uint32*)data)[2]), TYPE_COUNT));
		uint16 off = 3 * sizeof(uint32);
		rv->Assign(5, new StringVal(new BroString(data + off, Length() - off, 1)));
		}
		break;

	case IPPROTO_ESP:
		{
		rv = new RecordVal(hdrType(ip6_esp_type, "ip6_esp"));
		const uint32* esp = (const uint32*)data;
		rv->Assign(0, new Val(ntohl(esp[0]), TYPE_COUNT));
		rv->Assign(1, new Val(ntohl(esp[1]), TYPE_COUNT));
		}
		break;

#ifdef ENABLE_MOBILE_IPV6
	case IPPROTO_MOBILITY:
		{
		rv = new RecordVal(hdrType(ip6_mob_type, "ip6_mobility_hdr"));
		const struct ip6_mobility* mob = (const struct ip6_mobility*) data;
		rv->Assign(0, new Val(mob->ip6mob_payload, TYPE_COUNT));
		rv->Assign(1, new Val(mob->ip6mob_len, TYPE_COUNT));
		rv->Assign(2, new Val(mob->ip6mob_type, TYPE_COUNT));
		rv->Assign(3, new Val(mob->ip6mob_rsv, TYPE_COUNT));
		rv->Assign(4, new Val(ntohs(mob->ip6mob_chksum), TYPE_COUNT));

		RecordVal* msg = new RecordVal(hdrType(ip6_mob_msg_type, "ip6_mobility_msg"));
		msg->Assign(0, new Val(mob->ip6mob_type, TYPE_COUNT));

		uint16 off = sizeof(ip6_mobility);
		const u_char* msg_data = data + off;

		switch ( mob->ip6mob_type ) {
		case 0:
			{
			RecordVal* m = new RecordVal(hdrType(ip6_mob_brr_type, "ip6_mobility_brr"));
			m->Assign(0, new Val(ntohs(*((uint16*)msg_data)), TYPE_COUNT));
			off += sizeof(uint16);
			m->Assign(1, BuildOptionsVal(data + off, Length() - off));
			msg->Assign(1, m);
			}
			break;

		case 1:
			{
			RecordVal* m = new RecordVal(hdrType(ip6_mob_brr_type, "ip6_mobility_hoti"));
			m->Assign(0, new Val(ntohs(*((uint16*)msg_data)), TYPE_COUNT));
			m->Assign(1, new Val(ntohll(*((uint64*)(msg_data + sizeof(uint16)))), TYPE_COUNT));
			off += sizeof(uint16) + sizeof(uint64);
			m->Assign(2, BuildOptionsVal(data + off, Length() - off));
			msg->Assign(2, m);
			break;
			}

		case 2:
			{
			RecordVal* m = new RecordVal(hdrType(ip6_mob_brr_type, "ip6_mobility_coti"));
			m->Assign(0, new Val(ntohs(*((uint16*)msg_data)), TYPE_COUNT));
			m->Assign(1, new Val(ntohll(*((uint64*)(msg_data + sizeof(uint16)))), TYPE_COUNT));
			off += sizeof(uint16) + sizeof(uint64);
			m->Assign(2, BuildOptionsVal(data + off, Length() - off));
			msg->Assign(3, m);
			break;
			}

		case 3:
			{
			RecordVal* m = new RecordVal(hdrType(ip6_mob_brr_type, "ip6_mobility_hot"));
			m->Assign(0, new Val(ntohs(*((uint16*)msg_data)), TYPE_COUNT));
			m->Assign(1, new Val(ntohll(*((uint64*)(msg_data + sizeof(uint16)))), TYPE_COUNT));
			m->Assign(2, new Val(ntohll(*((uint64*)(msg_data + sizeof(uint16) + sizeof(uint64)))), TYPE_COUNT));
			off += sizeof(uint16) + 2 * sizeof(uint64);
			m->Assign(3, BuildOptionsVal(data + off, Length() - off));
			msg->Assign(4, m);
			break;
			}

		case 4:
			{
			RecordVal* m = new RecordVal(hdrType(ip6_mob_brr_type, "ip6_mobility_cot"));
			m->Assign(0, new Val(ntohs(*((uint16*)msg_data)), TYPE_COUNT));
			m->Assign(1, new Val(ntohll(*((uint64*)(msg_data + sizeof(uint16)))), TYPE_COUNT));
			m->Assign(2, new Val(ntohll(*((uint64*)(msg_data + sizeof(uint16) + sizeof(uint64)))), TYPE_COUNT));
			off += sizeof(uint16) + 2 * sizeof(uint64);
			m->Assign(3, BuildOptionsVal(data + off, Length() - off));
			msg->Assign(5, m);
			break;
			}

		case 5:
			{
			RecordVal* m = new RecordVal(hdrType(ip6_mob_brr_type, "ip6_mobility_bu"));
			m->Assign(0, new Val(ntohs(*((uint16*)msg_data)), TYPE_COUNT));
			m->Assign(1, new Val(ntohs(*((uint16*)(msg_data + sizeof(uint16)))) & 0x8000, TYPE_BOOL));
			m->Assign(2, new Val(ntohs(*((uint16*)(msg_data + sizeof(uint16)))) & 0x4000, TYPE_BOOL));
			m->Assign(3, new Val(ntohs(*((uint16*)(msg_data + sizeof(uint16)))) & 0x2000, TYPE_BOOL));
			m->Assign(4, new Val(ntohs(*((uint16*)(msg_data + sizeof(uint16)))) & 0x1000, TYPE_BOOL));
			m->Assign(5, new Val(ntohs(*((uint16*)(msg_data + 2*sizeof(uint16)))), TYPE_COUNT));
			off += 3 * sizeof(uint16);
			m->Assign(6, BuildOptionsVal(data + off, Length() - off));
			msg->Assign(6, m);
			break;
			}

		case 6:
			{
			RecordVal* m = new RecordVal(hdrType(ip6_mob_brr_type, "ip6_mobility_back"));
			m->Assign(0, new Val(*((uint8*)msg_data), TYPE_COUNT));
			m->Assign(1, new Val(*((uint8*)(msg_data + sizeof(uint8))) & 0x80, TYPE_BOOL));
			m->Assign(2, new Val(ntohs(*((uint16*)(msg_data + sizeof(uint16)))), TYPE_COUNT));
			m->Assign(3, new Val(ntohs(*((uint16*)(msg_data + 2*sizeof(uint16)))), TYPE_COUNT));
			off += 3 * sizeof(uint16);
			m->Assign(4, BuildOptionsVal(data + off, Length() - off));
			msg->Assign(7, m);
			break;
			}

		case 7:
			{
			RecordVal* m = new RecordVal(hdrType(ip6_mob_brr_type, "ip6_mobility_be"));
			m->Assign(0, new Val(*((uint8*)msg_data), TYPE_COUNT));
			const in6_addr* hoa = (const in6_addr*)(msg_data + sizeof(uint16));
			m->Assign(1, new AddrVal(IPAddr(*hoa)));
			off += sizeof(uint16) + sizeof(in6_addr);
			m->Assign(2, BuildOptionsVal(data + off, Length() - off));
			msg->Assign(8, m);
			break;
			}

		default:
			reporter->Weird(fmt("unknown_mobility_type_%d", mob->ip6mob_type));
			break;
		}

		rv->Assign(5, msg);
		}
		break;
#endif //ENABLE_MOBILE_IPV6

	default:
		break;
	}

	return rv;
	}
*/

static inline bool isIPv6ExtHeader(uint8 type)
	{
	switch (type) {
	case IPPROTO_HOPOPTS:
	case IPPROTO_ROUTING:
	case IPPROTO_DSTOPTS:
	case IPPROTO_FRAGMENT:
	case IPPROTO_AH:
	case IPPROTO_ESP:
#ifdef ENABLE_MOBILE_IPV6
	case IPPROTO_MOBILITY:
#endif
		return true;
	default:
		return false;
	}
	}

void IPv6_Hdr_Chain::Init(const struct ip6_hdr* ip6, int total_len,
                          bool set_next, uint16 next)
	{
	length = 0;
	uint8 current_type, next_type;
	next_type = IPPROTO_IPV6;
	const u_char* hdrs = (const u_char*) ip6;

	if ( total_len < (int)sizeof(struct ip6_hdr) )
		reporter->InternalError("IPv6_HdrChain::Init with truncated IP header");

	do
		{
		// We can't determine a given header's length if there's less than
		// two bytes of data available (2nd byte of extension headers is length)
		if ( total_len < 2 )
			return;

		current_type = next_type;
		IPv6_Hdr* p = new IPv6_Hdr(current_type, hdrs);

		next_type = p->NextHdr();
		uint16 cur_len = p->Length();

		// If this header is truncated, don't add it to chain, don't go further.
		if ( cur_len > total_len )
			{
			delete p;
			return;
			}

		if ( set_next && next_type == IPPROTO_FRAGMENT )
			{
			p->ChangeNext(next);
			next_type = next;
			}

		chain.push_back(p);

		// Check for routing headers and remember final destination address.
		if ( current_type == IPPROTO_ROUTING )
			ProcessRoutingHeader((const struct ip6_rthdr*) hdrs, cur_len);

#ifdef ENABLE_MOBILE_IPV6
		// Only Mobile IPv6 has a destination option we care about right now.
		if ( current_type == IPPROTO_DSTOPTS )
			ProcessDstOpts((const struct ip6_dest*) hdrs, cur_len);
#endif

		hdrs += cur_len;
		length += cur_len;
		total_len -= cur_len;

		} while ( current_type != IPPROTO_FRAGMENT &&
				  current_type != IPPROTO_ESP &&
#ifdef ENABLE_MOBILE_IPV6
				  current_type != IPPROTO_MOBILITY &&
#endif
				  isIPv6ExtHeader(next_type) );
	}

void IPv6_Hdr_Chain::ProcessRoutingHeader(const struct ip6_rthdr* r, uint16 len)
	{
	if ( finalDst )
		{
		// RFC 2460 section 4.1 says Routing should occur at most once.
		reporter->Weird(SrcAddr(), DstAddr(), "multiple_routing_headers");
		return;
		}

	// Last 16 bytes of header (for all known types) is the address we want.
	const in6_addr* addr = (const in6_addr*)(((const u_char*)r) + len - 16);

	switch ( r->ip6r_type ) {
	case 0: // Defined by RFC 2460, deprecated by RFC 5095
		{
		if ( r->ip6r_segleft > 0 && r->ip6r_len >= 2 )
			{
			if ( r->ip6r_len % 2 == 0 )
				finalDst = new IPAddr(*addr);
			else
				reporter->Weird(SrcAddr(), DstAddr(), "odd_routing0_len");
			}

		// Always raise a weird since this type is deprecated.
		reporter->Weird(SrcAddr(), DstAddr(), "routing0_hdr");
		}
		break;

#ifdef ENABLE_MOBILE_IPV6
	case 2: // Defined by Mobile IPv6 RFC 6275.
		{
		if ( r->ip6r_segleft > 0 )
			{
			if ( r->ip6r_len == 2 )
				finalDst = new IPAddr(*addr);
			else
				reporter->Weird(SrcAddr(), DstAddr(), "bad_routing2_len");
			}
		}
		break;
#endif

	default:
		reporter->Weird(fmt("unknown_routing_type_%d", r->ip6r_type));
		break;
	}
	}

#ifdef ENABLE_MOBILE_IPV6
void IPv6_Hdr_Chain::ProcessDstOpts(const struct ip6_dest* d, uint16 len)
	{
	const u_char* data = (const u_char*) d;
	len -= 2 * sizeof(uint8);
	data += 2* sizeof(uint8);

	while ( len > 0 )
		{
		const struct ip6_opt* opt = (const struct ip6_opt*) data;
		switch ( opt->ip6o_type ) {
		case 201: // Home Address Option, Mobile IPv6 RFC 6275 section 6.3
			{
			if ( opt->ip6o_len == 16 )
				if ( homeAddr )
					reporter->Weird(SrcAddr(), DstAddr(), "multiple_home_addr_opts");
				else
					homeAddr = new IPAddr(*((const in6_addr*)(data + 2)));
			else
				reporter->Weird(SrcAddr(), DstAddr(), "bad_home_addr_len");
			}
			break;

		default:
			break;
		}

		if ( opt->ip6o_type == 0 )
			{
			data += sizeof(uint8);
			len -= sizeof(uint8);
			}
		else
			{
			data += 2 * sizeof(uint8) + opt->ip6o_len;
			len -= 2 * sizeof(uint8) + opt->ip6o_len;
			}
		}
	}
#endif
/*
VectorVal* IPv6_Hdr_Chain::BuildVal() const
	{
	if ( ! ip6_ext_hdr_type )
		{
		ip6_ext_hdr_type = internal_type("ip6_ext_hdr")->AsRecordType();
		ip6_hopopts_type = internal_type("ip6_hopopts")->AsRecordType();
		ip6_dstopts_type = internal_type("ip6_dstopts")->AsRecordType();
		ip6_routing_type = internal_type("ip6_routing")->AsRecordType();
		ip6_fragment_type = internal_type("ip6_fragment")->AsRecordType();
		ip6_ah_type = internal_type("ip6_ah")->AsRecordType();
		ip6_esp_type = internal_type("ip6_esp")->AsRecordType();
		ip6_mob_type = internal_type("ip6_mobility_hdr")->AsRecordType();
		}

	VectorVal* rval = new VectorVal(
	    internal_type("ip6_ext_hdr_chain")->AsVectorType());

	for ( size_t i = 1; i < chain.size(); ++i )
		{
		RecordVal* v = chain[i]->BuildRecordVal();
		RecordVal* ext_hdr = new RecordVal(ip6_ext_hdr_type);
		uint8 type = chain[i]->Type();
		ext_hdr->Assign(0, new Val(type, TYPE_COUNT));

		switch (type) {
		case IPPROTO_HOPOPTS:
			ext_hdr->Assign(1, v);
			break;
		case IPPROTO_DSTOPTS:
			ext_hdr->Assign(2, v);
			break;
		case IPPROTO_ROUTING:
			ext_hdr->Assign(3, v);
			break;
		case IPPROTO_FRAGMENT:
			ext_hdr->Assign(4, v);
			break;
		case IPPROTO_AH:
			ext_hdr->Assign(5, v);
			break;
		case IPPROTO_ESP:
			ext_hdr->Assign(6, v);
			break;
#ifdef ENABLE_MOBILE_IPV6
		case IPPROTO_MOBILITY:
			ext_hdr->Assign(7, v);
			break;
#endif
		default:
			reporter->InternalError("IPv6_Hdr_Chain bad header %d", type);
			break;
		}
		rval->Assign(rval->Size(), ext_hdr, 0);
		}

	return rval;
	}
*/
