// See the file "COPYING" in the main distribution directory for copyright.

#ifndef TCP_H
#define TCP_H

//#include "analyzer.h"
#include "tcp.h"
#include "tcp_endpoint.h"
#include "tcp_reassembler.h"
//#include "packetdumper.h"
#include "ipaddr.h"
#include "reporter.h"

#include <string>
#include "config.h"

#ifndef WIN32
#define __FAVOR_BSD
#include <netinet/in.h>
#include <netinet/tcp.h>
//#include <ext/hash_map>
#include <map>
//#include <unordered_map>
#endif

#include "http.h"
#include "contentline.h"

class TCP_Analyzer;
class SSLWrapper_Analyzer;

#ifdef WIN32
typedef stdext::hash_map< std::string, TCP_Analyzer * >::iterator TCPConnectionsIterator;
#else
//typedef __gnu_cxx::hash_map< std::string, TCP_Analyzer * >::iterator TCPConnectionsIterator;
//typedef std::unordered_map< std::string, TCP_Analyzer * >::iterator TCPConnectionsIterator;
typedef std::map< std::string, TCP_Analyzer * >::iterator TCPConnectionsIterator;
#endif

typedef std::list< TCP_Analyzer * >::iterator TCPConnectionsExpirationIterator;


// We define two classes here:
// - TCP_Analyzer is the analyzer for the TCP protocol itself.
// - TCP_ApplicationAnalyzer is an abstract base class for analyzers for a
//   protocol running on top of TCP.

//class PIA_TCP;
//class TCP_ApplicationAnalyzer;
//class TCP_Reassembler;
//class HTTP_Analyzer;
//class ContentLine_Analyzer;

class TCP_Flags {
public:
	TCP_Flags(const struct tcphdr* tp)	{ flags = tp->th_flags; }

	bool SYN() const	{ return !!(flags & TH_SYN); }
	bool FIN() const	{ return !!(flags & TH_FIN); }
	bool RST() const	{ return !!(flags & TH_RST); }
	bool ACK() const	{ return !!(flags & TH_ACK); }
	bool URG() const	{ return !!(flags & TH_URG); }
	bool PUSH() const	{ return !!(flags & TH_PUSH); }

protected:
	u_char flags;
};

class TCP_Analyzer /* : public TransportLayerAnalyzer */ {
public:
	TCP_Analyzer(const IPAddr& src_addr, const int src_port, const IPAddr& dst_addr, const int dst_port);
	virtual ~TCP_Analyzer();
	void Reset(const IPAddr& src_addr, const int src_port, const IPAddr& dst_addr, const int dst_port);
	void Dump();
	virtual void Done();

	void SetSkip(bool do_skip)		{ skip = do_skip; }
	bool Skipping() const			{ return skip; }

	double StartTime() const		{ return start_time; }
	void  SetStartTime(double t)		{ start_time = t; }
	double LastTime() const			{ return last_time; }
	void SetLastTime(double t) 		{ last_time = t; }

	const IPAddr& OrigAddr() const		{ return orig_addr; }
	const IPAddr& RespAddr() const		{ return resp_addr; }

	uint32 OrigPort() const			{ return orig_port; }
	uint32 RespPort() const			{ return resp_port; }

	void SetContainerIterator(TCPConnectionsIterator & itr)
	{
		tcp_itr = itr;
	}
	TCPConnectionsIterator & GetContainerIterator()
	{
		return tcp_itr;
	}
	void SetExpirationQueueIterator(TCPConnectionsExpirationIterator & itr)
	{
		tcp_exp_itr = itr;
	}
	TCPConnectionsExpirationIterator & GetExpirationQueueIterator()
	{
		return tcp_exp_itr;
	}
	bool IsExpired(double t)
	{
		return (t > expiration);
	}
	bool IsEncrypted()
	{
		return (ssl != 0);
	}
	void GotReply()
	{
		if ( nrequests > 0 )
		{
			nrequests = 0;
		}
		// reset partial flag if we have one!
		is_partial = 0;
	}
	void GotRequest(const char * req)
	{
		if (nrequests)
		{
			if (strcmp(req, "GET") == 0)
				Weird("DDDDDD: we have already not finished request here[GET]");
			else
				Weird("DDDDDD: we have already not finished request here[POST]");
		}
		nrequests=1;
	}
/*
	// Returns true if the history was already seen, false otherwise.
	int CheckHistory(uint32 mask, char code)
		{
		if ( (hist_seen & mask) == 0 )
			{
			hist_seen |= mask;
			AddHistory(code);
			return false;
			}
		else
			return true;
		}

	void AddHistory(char code)	{ history += code; }
*/

	// Add a child analyzer that will always get the packets,
	// independently of whether we do any reassembly.
//	void AddChildPacketAnalyzer(Analyzer* a)
//		{ packet_children.push_back(a); a->SetParent(this); }

	// True if the connection has closed in some sense, false otherwise.
	int IsClosed() const	{ return orig.did_close || resp.did_close; }
	int BothClosed() const	{ return orig.did_close && resp.did_close; }

	int IsPartial() const	{ return is_partial; }

	bool HadGap(bool orig) const;

	const TCP_Endpoint* Orig() const	{ return &orig; }
	const TCP_Endpoint* Resp() const	{ return &resp; }
	int OrigState() const	{ return orig.state; }
	int RespState() const	{ return resp.state; }
	int OrigPrevState() const	{ return orig.prev_state; }
	int RespPrevState() const	{ return resp.prev_state; }
	uint32 OrigSeq() const	{ return orig.LastSeq(); }
	uint32 RespSeq() const	{ return resp.LastSeq(); }
	bool IsFullyClosed() { return (orig.did_close && resp.did_close) || 
		orig.state == TCP_ENDPOINT_RESET || 
		resp.state == TCP_ENDPOINT_RESET; }

	// True if either endpoint still has pending data.  closing_endp
	// is an endpoint that has indicated it is closing (i.e., for
	// which we have seen a FIN) - for it, data is pending unless
	// everything's been delivered up to the FIN.  For its peer,
	// the test is whether it has any outstanding, un-acked data.
	int DataPending(TCP_Endpoint* closing_endp);

//	virtual void SetContentsFile(unsigned int direction, BroFile* f);
//	virtual BroFile* GetContentsFile(unsigned int direction) const;

	// Callback to process a TCP option.
	typedef int (*proc_tcp_option_t)(unsigned int opt, unsigned int optlen,
			const u_char* option, TCP_Analyzer* analyzer,
			bool is_orig, void* cookie);

	// From Analyzer.h
//	virtual void UpdateConnVal(RecordVal *conn_val);

	// Needs to be static because it's passed as a pointer-to-function
	// rather than pointer-to-member-function.
//	static int ParseTCPOptions(const struct tcphdr* tcp,
//			proc_tcp_option_t proc, TCP_Analyzer* analyzer,
//			bool is_orig, void* cookie);

//	static Analyzer* InstantiateAnalyzer(Connection* conn)
//		{ return new TCP_Analyzer(conn); }

	static bool Available()	{ return true; }

	void Weird(const char * msg)
	{
		fprintf(stdout, "weird: %s:%d => %s:%d %s\n", orig_addr.AsURIString().c_str(), ntohs(orig_port), resp_addr.AsURIString().c_str(), ntohs(resp_port), msg);
		//reporter->Weird(msg);
	}
	void DeliverPacket(int len, const u_char* data, const IP_Hdr* ip, int caplen);
protected:
	//friend class TCP_ApplicationAnalyzer;
	friend class TCP_Reassembler;
	//friend class PIA_TCP;

	// Analyzer interface.
	virtual void Init();
	virtual void DeliverStream(int len, const u_char* data, bool orig);
	virtual void Undelivered(int seq, int len, bool orig);
	//virtual void FlipRoles();
	virtual bool IsReuse(double t, const u_char* pkt);

	// Returns the TCP header pointed to by data (which we assume is
	// aligned), updating data, len & caplen.  Returns nil if the header
	// isn't fully present.
	const struct tcphdr* ExtractTCP_Header(const u_char*& data, int& len,
						int& caplen);

	// Returns true if the checksum is valid, false if not (and in which
	// case also updates the status history of the endpoint).
	bool ValidateChecksum(const struct tcphdr* tp, TCP_Endpoint* endpoint,
				int len, int caplen);

	// Update analysis based on flag combinations.  The endpoint, base_seq
	// and len are needed for tracking various history information.
	// dst_port is needed for trimming of FIN packets.
	void CheckFlagCombos(const TCP_Flags & flags, TCP_Endpoint* endpoint,
				uint32 base_seq, int len, int dst_port);

	void UpdateWindow(TCP_Endpoint* endpoint, unsigned int window,
					uint32 base_seq, uint32 ack_seq,
					const TCP_Flags & flags);

	void ProcessSYN(const IP_Hdr* ip, const struct tcphdr* tp,
			uint32 tcp_hdr_len, int& seq_len,
			TCP_Endpoint* endpoint, TCP_Endpoint* peer,
			uint32 base_seq, uint32 ack_seq,
			const IPAddr& orig_addr,
			bool is_orig, const TCP_Flags & flags);

	void ProcessFIN(double t, TCP_Endpoint* endpoint, int& seq_len,
			uint32 base_seq);

	bool ProcessRST(double t, TCP_Endpoint* endpoint, const IP_Hdr* ip,
			uint32 base_seq, int len, int& seq_len);

	void ProcessACK(TCP_Endpoint* endpoint, TCP_Endpoint* peer,
			uint32 ack_seq, int is_orig, const TCP_Flags & flags, int len);

	int ProcessFlags(double t, const IP_Hdr* ip, const struct tcphdr* tp,
			uint32 tcp_hdr_len, int len, int& seq_len,
			TCP_Endpoint* endpoint, TCP_Endpoint* peer,
			uint32 base_seq, uint32 ack_seq,
			const IPAddr& orig_addr,
			bool is_orig, const TCP_Flags & flags);

	void TransitionFromInactive(double t, TCP_Endpoint* endpoint,
					uint32 base_seq, uint32 last_seq,
					int SYN);

	// Update the state machine of the TCPs based on the activity.  This
	// includes our pseudo-states such as TCP_ENDPOINT_PARTIAL.
	//
	// On return, do_close is true if we should consider the connection
	// as closed, and gen_event if we shouuld generate an event about
	// this fact.
	void UpdateStateMachine(double t,
			TCP_Endpoint* endpoint, TCP_Endpoint* peer,
			uint32 base_seq, uint32 ack_seq, uint32 last_seq,
			int len, int delta_last, int is_orig, const TCP_Flags & flags,
			int& do_close, int& gen_event);

	void UpdateInactiveState(double t,
				TCP_Endpoint* endpoint, TCP_Endpoint* peer,
				uint32 base_seq, uint32 ack_seq,
				int len, int is_orig, const TCP_Flags & flags,
				int& do_close, int& gen_event);

	void UpdateSYN_SentState(double t,
				TCP_Endpoint* endpoint, TCP_Endpoint* peer,
				uint32 base_seq, uint32 last_seq,
				int len, int is_orig, const TCP_Flags & flags,
				int& do_close, int& gen_event);

	void UpdateEstablishedState(double t,
				TCP_Endpoint* endpoint, TCP_Endpoint* peer,
				uint32 base_seq, uint32 last_seq,
				int is_orig, const TCP_Flags & flags,
				int& do_close, int& gen_event);

	void UpdateClosedState(double t, TCP_Endpoint* endpoint,
				int delta_last, const TCP_Flags & flags,
				int& do_close);

	void UpdateResetState(int len, const TCP_Flags & flags);

	int DeliverData(double t, const u_char* data, int len, int caplen,
			const IP_Hdr* ip, const struct tcphdr* tp,
			TCP_Endpoint* endpoint, uint32 base_seq,
			bool is_orig, const TCP_Flags & flags);

	void CheckRecording(int need_contents, const TCP_Flags & flags);
	void CheckPIA_FirstPacket(int is_orig, const IP_Hdr* ip);

	// Returns the difference between last_seq and the last sequence
	// seen by the endpoint (may be negative).
	int UpdateLastSeq(TCP_Endpoint* endpoint, uint32 last_seq,
				const TCP_Flags & flags);

	friend class ConnectionTimer;
	void AttemptTimer(double t);
	void PartialCloseTimer(double t);
	void ExpireTimer(double t);
	void ResetTimer(double t);
	void DeleteTimer(double t);
//	void ConnDeleteTimer(double t)	{ Conn()->DeleteTimer(t); }

	void EndpointEOF(TCP_Reassembler* endp);
	void ConnectionClosed(TCP_Endpoint* endpoint,
					TCP_Endpoint* peer, int gen_event);
	void ConnectionFinished(int half_finished);
	void ConnectionReset();
	void PacketWithRST();

//	Val* BuildSYNPacketVal(int is_orig,
//				const IP_Hdr* ip, const struct tcphdr* tcp);

//	RecordVal* BuildOSVal(int is_orig, const IP_Hdr* ip,
//				const struct tcphdr* tcp, uint32 tcp_hdr_len);

	// Needs to be static because it's passed as a pointer-to-function
	// rather than pointer-to-member-function.
//	static int TCPOptionEvent(unsigned int opt, unsigned int optlen,
//				const u_char* option, TCP_Analyzer* analyzer,
//				  bool is_orig, void* cookie);

private:
        IPAddr orig_addr;
        IPAddr resp_addr;
        uint32 orig_port;
        uint32 resp_port;       // in network order

	TCP_Endpoint orig;
	TCP_Endpoint resp;
	TCP_Reassembler orig_reassembler;
	TCP_Reassembler resp_reassembler;
	SSLWrapper_Analyzer * ssl;

	bool skip;
	double start_time;
	double last_time;

	//std::string history;
	uint32 hist_seen;
	ContentLine_Analyzer content_line_orig;
	ContentLine_Analyzer content_line_resp;
	HTTP_Analyzer http;
	TCPConnectionsIterator tcp_itr;
	TCPConnectionsExpirationIterator tcp_exp_itr;
	double expiration;

//	analyzer_list packet_children;

	unsigned int first_packet_seen: 2;
	//unsigned int reassembling: 1;
	unsigned int is_partial: 1;
	unsigned int is_active: 1;
	unsigned int finished: 1;

	// Whether we're waiting on final data delivery before closing
	// this connection.
	unsigned int close_deferred: 1;

	// Whether to generate an event when we finally do close it.
	unsigned int deferred_gen_event: 1;

	// Whether we have seen the first ACK from the originator.
	unsigned int seen_first_ACK: 1;
	// Debug
	unsigned int nrequests;
};

/*
class TCP_ApplicationAnalyzer : public Analyzer {
public:
	TCP_ApplicationAnalyzer(AnalyzerTag::Tag tag, Connection* conn)
	: Analyzer(tag, conn)
		{ tcp = 0; }

	virtual ~TCP_ApplicationAnalyzer()	{ }

	// This may be nil if we are not directly associated with a TCP
	// analyzer (e.g., we're part of a tunnel decapsulation pipeline).
	TCP_Analyzer* TCP()
		{
		return tcp ?
			tcp :
			static_cast<TCP_Analyzer*>(
				Conn()->FindAnalyzer(AnalyzerTag::TCP));
		}

	void SetTCP(TCP_Analyzer* arg_tcp)	{ tcp = arg_tcp; }

	// The given endpoint's data delivery is complete.
	virtual void EndpointEOF(bool is_orig);

	// Called whenever an end enters TCP_ENDPOINT_CLOSED or
	// TCP_ENDPOINT_RESET.  If gen_event is true and the connection
	// is now fully closed, a connection_finished event will be
	// generated; otherwise not.
	virtual void ConnectionClosed(TCP_Endpoint* endpoint,
					TCP_Endpoint* peer, int gen_event);
	virtual void ConnectionFinished(int half_finished);
	virtual void ConnectionReset();

	// Called whenever a RST packet is seen - sometimes the invocation
	// of ConnectionReset is delayed.
	virtual void PacketWithRST();

	virtual void DeliverPacket(int len, const u_char* data, bool orig,
					int seq, const IP_Hdr* ip, int caplen);
	virtual void Init();

	// This suppresses violations if the TCP connection wasn't
	// fully established.
	virtual void ProtocolViolation(const char* reason,
					const char* data = 0, int len = 0);

	// "name" and "val" both now belong to this object, which needs to
	//  delete them when done with them.
	virtual void SetEnv(bool orig, char* name, char* val);

protected:
   	TCP_ApplicationAnalyzer() 	{ };

private:
	TCP_Analyzer* tcp;
};
*/

/*
class TCP_SupportAnalyzer : public SupportAnalyzer {
public:
	TCP_SupportAnalyzer(AnalyzerTag::Tag tag, Connection* conn, bool arg_orig)
		: SupportAnalyzer(tag, conn, arg_orig)	{ }

	virtual ~TCP_SupportAnalyzer() {}

	// These are passed on from TCP_Analyzer.
	virtual void EndpointEOF(bool is_orig)	{ }
	virtual void ConnectionClosed(TCP_Endpoint* endpoint,
					TCP_Endpoint* peer, int gen_event) 	{ }
	virtual void ConnectionFinished(int half_finished)	{ }
	virtual void ConnectionReset()	{ }
	virtual void PacketWithRST()	{ }
};
*/

#endif
