// See the file "COPYING" in the main distribution directory for copyright.

#ifndef frag_h
#define frag_h

//#include "util.h"
#include "ip.h"
//#include "Net.h"
#include "reassem.h"
//#include "timer.h"

#include <list>
#ifndef WIN32
//#include <ext/hash_map>
#include <map>
//#include <unordered_map>
#endif

class FragReassembler;

#ifdef WIN32
typedef stdext::hash_map< std::string, FragReassembler * >::iterator FragmentsIterator;
#else
//typedef __gnu_cxx::hash_map< std::string, FragReassembler * >::iterator FragmentsIterator;
//typedef std::unordered_map< std::string, FragReassembler * >::iterator FragmentsIterator;
typedef std::map< std::string, FragReassembler * >::iterator FragmentsIterator;
#endif

typedef std::list< FragReassembler * >::iterator FragmentsExpirationIterator;

//class FragTimer;

typedef void (FragReassembler::*frag_timer_func)(double t);

/**
 * FragReassembler class is used during IP defragmentation. It is almost not changed from the original BRO version.
 *
 */
class FragReassembler : public Reassembler {
public:
	/// Object constructor
	FragReassembler(/*NetSessions* s, */ const IP_Hdr* ip, const u_char* pkt,
			/* HashKey* k,*/ double t);
	/// Object destructor.
	~FragReassembler();

	/// Add a new IP packet and try to defragment all fragmented packets.
	void AddFragment(double t, const IP_Hdr* ip, const u_char* pkt);

	/// Return defragmented IP packet.
	const IP_Hdr* ReassembledPkt()	{ return reassembled_pkt; }

	/// Saves object position to this class in the list of active fragmented IP packets.
	void SetContainerIterator(FragmentsIterator & itr)
	{
		frag_itr = itr;
	}
	/// Returns object position in the list of active fragmented IP packets.
	FragmentsIterator & GetContainerIterator()
	{
		return frag_itr;
	}
	/// Saves object position to this class in the expiration queue of fragmented IP packets.
	void SetExpirationQueueIterator(FragmentsExpirationIterator & itr)
	{
		frag_exp_itr = itr;
	}
	/// Returns object position to this class in the expiration queue.
	FragmentsExpirationIterator & GetExpirationQueueIterator()
	{
		return frag_exp_itr;
	}
	/// Check if object is expired.
	bool IsExpired(double t)
	{
		return (t >= expiration);
	}

protected:
	void BlockInserted(DataBlock* start_block);
	void Overlap(const u_char* b1, const u_char* b2, int n);

	u_char* proto_hdr;
	IP_Hdr* reassembled_pkt;
	int proto_hdr_len;
	//NetSessions* s;
	int frag_size;	// size of fully reassembled fragment
	uint16 next_proto; // first IPv6 fragment header's next proto field
	//HashKey* key;

	/// Position to this class in the list of active fragmented IP packets
	FragmentsIterator frag_itr;
	/// Position to this class in the expiration queue of fragmented IP packets.
	FragmentsExpirationIterator frag_exp_itr;
	/// Packet expected expiration time
	double expiration;
};


#endif
