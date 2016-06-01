

// build
// g++ -g ./pcap-filter.cpp -lpcap -o pcap-filter


//
// 41711 /home/hybrid/src/atms3/dump.pcap

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>

#ifndef WIN32
#define __FAVOR_BSD
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#endif

#define DEBUG_MSG printf

double network_time;
int readfile(const char * fname);
class IP_Hdr;
void DoNextPacket(double t, const struct pcap_pkthdr* hdr, const u_char *pkt_data,
                                const IP_Hdr* ip_hdr, const u_char* const pkt,
                                int hdr_size);
void dispatcher_handler(u_char *temp1, const struct pcap_pkthdr *header, const u_char *pkt_data);
int global_port = 80;
pcap_t *fp;
pcap_dumper_t *fpwrite;
static int datalink = 0;
static unsigned int headersize = 0;

class IP_Hdr {
public:
        IP_Hdr(const struct ip* arg_ip4, bool arg_del)
                : ip4(arg_ip4)
                {
                }
        /**
         * If an IPv4 packet is wrapped, return a pointer to it, else null.
         */
        const struct ip* IP4_Hdr() const        { return ip4; }
        const u_char* Payload() const
                {
                        return ((const u_char*) ip4) + ip4->ip_hl * 4;
                }
        unsigned int TotalLen() const
                { return ntohs(ip4->ip_len);  }
        unsigned int HdrLen() const
                { return ip4->ip_hl * 4 ; }
        unsigned char NextProto() const
                { return ip4->ip_p; }
private:
        const struct ip* ip4;
        const struct ip6_hdr* ip6;
};

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Program usage:\n");
		printf("Usage: pcap-filter port pcap-file\n\n");
		exit(0);
	}
	printf("starting\n");

	printf("port: %d\n", atoi(argv[1]));
	global_port = ntohs(atoi(argv[1]));
	readfile(argv[2]);	
}

int get_link_header_size(int dl)
        {
        switch ( dl ) {
        case DLT_NULL:
                return 4;

        case DLT_EN10MB:
                return 14;

        case DLT_FDDI:
                return 13 + 8;  // fddi_header + LLC

#ifdef DLT_LINUX_SLL
        case DLT_LINUX_SLL:
                return 16;
#endif

        case DLT_PPP_SERIAL:    // PPP_SERIAL
                return 4;

        case DLT_RAW:
                return 0;
        }

        return -1;
}

int readfile(const char * fname)
{
	char out_file[1024];
        pcap_t *fp;
        char errbuf[PCAP_ERRBUF_SIZE];
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

	sprintf(out_file, "./pcap-out-%u.pcap", global_port);
	printf("writing data to: %s\n", out_file);
	fpwrite = pcap_dump_open(fp, out_file);
        /* read and dispatch packets until EOF is reached */
        pcap_loop(fp, 0, dispatcher_handler, NULL);
	pcap_dump_close(fpwrite);
        pcap_close(fp);
        return 0;
}

void dispatcher_handler(u_char *temp1, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
        const unsigned char * data = pkt_data;

        unsigned int pkt_hdr_size = headersize;

        // Unfortunately some packets on the link might have MPLS labels
        // while others don't. That means we need to ask the link-layer if
        // labels are in place.
        bool have_mpls = false;

        int protocol = 0;
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
                        DEBUG_MSG("in vlan !\n");
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
        const struct ip* ip_hdr = 0;
        const unsigned char * ip_data = 0;
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
        ip_data = data + (ip_hdr->ip_hl << 2);

        unsigned int caplen = header->caplen - pkt_hdr_size;

        if ( ip_hdr->ip_v == 4 )
        {
                //printf("going to DoNextPacket\n");
                IP_Hdr ip_hdr2( ip_hdr, false);
                DoNextPacket(network_time, header, pkt_data, &ip_hdr2, data, pkt_hdr_size);
        }
        else if ( ip_hdr->ip_v == 6 )
        {
                DEBUG_MSG("we have ipv6\n");
        } else {
                DEBUG_MSG("unknown ip format %d\n", ip_hdr->ip_v);
        }
        return;
}

void DoNextPacket(double t, const struct pcap_pkthdr* hdr, const u_char *pkt_data,
                                const IP_Hdr* ip_hdr, const u_char* const pkt,
                                int hdr_size)
        {
        unsigned int caplen = hdr->caplen - hdr_size;
        const struct ip* ip4 = ip_hdr->IP4_Hdr();

        unsigned int len = ip_hdr->TotalLen();
        if ( hdr->len < len + hdr_size )
                {
                DEBUG_MSG("truncated_IP");
                return;
                }

        int ip_hdr_len = ip_hdr->HdrLen();

        len -= ip_hdr_len;      // remove IP header
        caplen -= ip_hdr_len;
        int proto = ip_hdr->NextProto();

	if (proto != IPPROTO_TCP)
		return;

        const u_char* data = ip_hdr->Payload();
	const struct tcphdr* tp = (const struct tcphdr *) data;
        unsigned int src_port = tp->th_sport;
        unsigned int dst_port = tp->th_dport;
	if (src_port != global_port && dst_port != global_port)
		return;
#ifdef BRO_DEBUG
                DEBUG_MSG("%s:%d => %s:%d : %s%s%s%s\n",
                        ip_hdr->SrcAddr().AsURIString().c_str(), ntohs(src_port),
                        ip_hdr->DstAddr().AsURIString().c_str(), ntohs(dst_port),
                        (tp->th_flags & TH_SYN) ? "S ": "",
                        (tp->th_flags & TH_FIN) ? "F ": "",
                        (tp->th_flags & TH_RST) ? "R ": "",
                        (tp->th_flags & TH_URG) ? "U ": ""
                        );
#endif
       pcap_dump((unsigned char *)fpwrite, hdr, pkt_data);
}
