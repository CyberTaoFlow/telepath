#include <algorithm>

#include "contentline.h"
#include "http.h"
#include "tcp.h"

/**
 * Object constructor.
 */
ContentLine_Analyzer::ContentLine_Analyzer(HTTP_Analyzer * _http /* Connection* conn */, bool _orig )
/* : TCP_SupportAnalyzer(AnalyzerTag::ContentLine, conn, orig) */
	{
	http = _http;
	orig = _orig;
	buf = NULL;
	buf_len = 0;
	Reset();
	InitBuffer(0);
	}
/**
 * Reset() function is used when object is reused. All internal variables are changed to default values.
 */
void ContentLine_Analyzer::Reset()
	{
	flag_NULs = 0;
	CR_LF_as_EOL = (CR_as_EOL | LF_as_EOL);
	skip_deliveries = 0;
	skip_partial = 0;
	this->seq_delivered_in_lines = 0;
	this->skip_pending = 0;
	this->seq = 0;
	this->seq_to_skip = 0;
	plain_delivery_length = 0;
	is_plain = false;
	offset = 0;
	last_char = 0;
	}

/**
 * Init internal buffer used to store tokens (lines from HTTP header)
 */
void ContentLine_Analyzer::InitBuffer(size_t size)
	{
	if ( buf && buf_len >= size )
		// Don't shrink the buffer, because it's not clear in that
		// case how to deal with characters in it that no longer fit.
		return;

	if ( size < sizeof(default_line) - 1 )
	{
		buf = (unsigned char *) &default_line;
		offset = 0;
		buf_len = size = sizeof(default_line) - 1;
		return;
	}
	else
		size += 1000;

	u_char* b = new u_char[size];
	//u_char* b = (unsigned char*)malloc(size);
	if ( ! b )
	{
		reporter->InternalError("out of memory delivering endpoint line");
		return;
	}

	if ( buf )
		{
		if ( offset > 0 )
			memcpy(b, buf, offset);
		//free(buf);
		if (buf != (unsigned char*)&default_line)
			delete [] buf;
		}
	else
		{
		offset = 0;
		last_char = 0;
		}

	buf = b;
	buf_len = size;
	}

/**
 * Object destructor
 */
ContentLine_Analyzer::~ContentLine_Analyzer()
	{
	if (buf && buf != (unsigned char*)&default_line)
		{
		//free (buf );
		delete [] buf;
		}
	}

/**
 * Check if we are inside a not finished token.
 */
int ContentLine_Analyzer::HasPartialLine() const
	{
	return buf && offset > 0;
	}

/**
 * DeliverStream() is callsed from the TCP reassembly or from SSL decryption when new data is available.
 */
void ContentLine_Analyzer::DeliverStream(int len, const unsigned char* data,
						bool is_orig)
	{
//	TCP_SupportAnalyzer::DeliverStream(len, data, is_orig);
	//printf("ContentLine_Analyzer::DeliverStream: buf size(%d), offset(%d), len(%d)\n", buf_len, offset, len );

	if ( len <= 0 || SkipDeliveries() )
		return;

	if ( skip_partial )
		{
		if ( http && http->tcp->IsPartial() )
			return;
		}

	// Make sure we have enough room to accommodate the new stuff.
//	if ( len + offset >= buf_len )
//		InitBuffer( len + offset );

	DoDeliver(len, data);

	// bug was here - Yuli
	// we increment seq on each parsed request line
	//seq += len;
	}

/**
 * Undelivered() function is called when we have some missing TCP packets.
 */
void ContentLine_Analyzer::Undelivered(int seq, int len, bool orig)
	{
	DEBUG_MSG("undelivered content \n");
	http->Undelivered(seq, len, orig, this);
	}

void ContentLine_Analyzer::EndpointEOF(bool is_orig)
	{
	if ( offset > 0 )
		DeliverStream(1, (const unsigned char*) "\n", is_orig);
	}

/**
 * SetPlainDelivery() function is called when we are parsing for example Content-line value.
 *
 * @param length specifies expected size of data.
 */
void ContentLine_Analyzer::SetPlainDelivery(int64_t length)
	{
	if ( length < 0 )
		reporter->InternalError("negative length for plain delivery");

	plain_delivery_length = length;
	}

/**
 * HTTP header parser.
 */
void ContentLine_Analyzer::DoDeliver(int len, const u_char* data)
	{
	this->seq_delivered_in_lines = this->seq;

	while ( len > 0 && ! SkipDeliveries() )
		{
		if ( (CR_LF_as_EOL & CR_as_EOL) &&
		     last_char == '\r' && *data == '\n' )
			{
			// CR is already considered as EOL.
			// Compress CRLF to just one line termination.
			//
			// Note, we test this prior to checking for
			// "plain delivery" because (1) we might have
			// made the decision to switch to plain delivery
			// based on a line terminated with '\r' for
			// which a '\n' then arrived, and (2) we are
			// careful when executing plain delivery to
			// clear last_char once we do so.
			last_char = *data;
			--len; ++data; ++this->seq;
			++this->seq_delivered_in_lines;
			}

		if ( plain_delivery_length > 0 )
			{
			int deliver_plain = min(plain_delivery_length, (int64_t)len);

			last_char = 0; // clear last_char
			plain_delivery_length -= deliver_plain;

			is_plain = true;

			http->DeliverStream(deliver_plain, data, IsOrig(), this);

			is_plain = false;

			data += deliver_plain;
			len -= deliver_plain;
			// added next line as a bug fixes - Yuli
			this->seq += deliver_plain;
			if ( len == 0 )
				return;
			}

		if ( this->skip_pending > 0 )
			SkipBytes(this->skip_pending);

		// Note that the skipping must take place *after*
		// the CR/LF check above, so that the '\n' of the
		// previous line is skipped first.
		if ( this->seq < this->seq_to_skip )
			{
			// Skip rest of the data and return
			int64_t skip_len = this->seq_to_skip - this->seq;
			if ( skip_len > len )
				skip_len = len;

			DEBUG_MSG("undelivered content2 \n");
			http->Undelivered(this->seq, skip_len, IsOrig(), this);

			len -= skip_len; data += skip_len; this->seq += skip_len;
			this->seq_delivered_in_lines += skip_len;
			}

		if ( len <= 0 )
			break;

		if (orig == true && http->request_state == EXPECT_REQUEST_RESET)
		{
			offset = 0;
			http->request_state = EXPECT_REQUEST_LINE;
		}
		// sanity check 1
		if (orig == true && len > 0)
		{
			if (offset != 0)
			{
				if (strncmp((const char*)data, "GET /",5) == 0)
				{
					http->Weird("XXXXXX: GET / request broken !");
				} else if (strncmp((const char*)data, "POST /",6) == 0)
				{
					http->Weird("XXXXXX: POST / request broken !");
				}
			}
			if (http->request_state != EXPECT_REQUEST_LINE)
			{
				if (strncmp((const char*)data, "GET /",5) == 0)
				{
					http->Weird("ZZZZZZ: GET / request broken !");
				} else if (strncmp((const char*)data, "POST /",6) == 0)
				{
					http->Weird("ZZZZZZ: POST / request broken !");
				}
			}

		}

		int n = DoDeliverOnce(len, data);
		// check if we failed to parse correct http header line
		// it can happen when some packets in response were underlivered
		// we will skip this packet
		// reply state will be changed to correct one on next received HTTP request
		if (orig == false && http->reply_state == EXPECT_REPLY_NOTHING)
		{
			data += len;
			this->seq += len;
			len = 0;
			offset = 0;
			return;
		}
		len -= n;
		data += n;
		this->seq += n;
		}
	}

/**
 * Deliver one token back to HTTP class using http->DeliverStream() function.
 */
int ContentLine_Analyzer::DoDeliverOnce(int len, const u_char* data)
	{
	const u_char* data_start = data;

	if ( len <= 0 )
		return 0;

	for ( ; len > 0; --len, ++data )
		{
		if ( offset >= buf_len )
			InitBuffer( buf_len + offset );

		int c = data[0];

#define EMIT_LINE \
	{ \
	buf[offset] = '\0'; \
	int seq_len = data + 1 - data_start; \
	seq_delivered_in_lines = seq + seq_len; \
	last_char = c; \
	http->DeliverStream(offset, buf, IsOrig(), this); \
	offset = 0; \
	return seq_len; \
	}

		switch ( c ) {
		case '\r':
			// Look ahead for '\n'.
			if ( len > 1 && data[1] == '\n' )
				{
				--len; ++data;
				last_char = c;
				c = data[0];
				EMIT_LINE
				}

			else if ( CR_LF_as_EOL & CR_as_EOL )
				EMIT_LINE

			else
				buf[offset++] = c;
			break;

		case '\n':
			if ( last_char == '\r' ){
				--offset; // remove '\r'
				EMIT_LINE
			}

			else if ( CR_LF_as_EOL & LF_as_EOL ){
				//Request Smuggling Bug.

				//EMIT_LINE
			}
			else{
//				if ( Conn()->FlagEvent(SINGULAR_LF) )
//					Conn()->Weird("line_terminated_with_single_LF");
				buf[offset++] = c;
			}
			break;

		case '\0':
			if ( flag_NULs )
				CheckNUL();
			else
				buf[offset++] = c;
			break;

		default:
			buf[offset++] = c;
			break;
		}

		if ( last_char == '\r' )
			{
			//			if ( Conn()->FlagEvent(SINGULAR_CR) )
			//				Conn()->Weird("line_terminated_with_single_CR");
			}

		last_char = c;
		}

	return data - data_start;
	}

void ContentLine_Analyzer::CheckNUL()
	{
	// If this is the first byte seen on this connection,
	// and if the connection's state is PARTIAL, then we've
	// intercepted a keep-alive, and shouldn't complain
	// about it.  Note that for PARTIAL connections, the
	// starting sequence number is adjusted as though there
	// had been an initial SYN, so we check for whether
	// the connection has at most two bytes so far.

//	TCP_Analyzer* tcp =
//		static_cast<TCP_ApplicationAnalyzer*>(Parent())->TCP();

	if ( http )
		{
		TCP_Analyzer * tcp = http->tcp;
		const TCP_Endpoint* endp = IsOrig() ? tcp->Orig() : tcp->Resp();
		if ( endp->state == TCP_ENDPOINT_PARTIAL &&
		     endp->LastSeq() - endp->StartSeq() <= 2 )
			; // Ignore it.
		else
			{
//			if ( Conn()->FlagEvent(NUL_IN_LINE) )
//				Conn()->Weird("NUL_in_line");
			flag_NULs = 0;
			}
		}
	}

void ContentLine_Analyzer::SkipBytesAfterThisLine(int64_t length)
	{
	// This is a little complicated because Bro has to handle
	// both CR and CRLF as a line break. When a line is delivered,
	// it's possible that only a CR is seen, and we may not know
	// if an LF is following until we see the next packet.  If an
	// LF follows, we should start skipping bytes *after* the LF.
	// So we keep the skip as 'pending' until we see the next
	// character in DoDeliver().

	if ( last_char == '\r' )
		this->skip_pending = length;
	else
		SkipBytes(length);
	}

void ContentLine_Analyzer::SkipBytes(int64_t length)
	{
	this->skip_pending = 0;
	this->seq_to_skip = SeqDelivered() + length;
	}

