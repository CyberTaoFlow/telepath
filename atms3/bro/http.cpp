// See the file "COPYING" in the main distribution directory for copyright.

#include "config.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <map>

//#include "NetVar.h"
#include "http.h"
//#include "event.h"
#include "mime.h"
#include "tcp.h"
#include "zip.h"

const bool DEBUG_http = false;
const int mime_segment_length = 4000;
const int http_entity_data_delivery_size = max_post_size;
namespace BifConst { const bool skip_http_data = false; }
const int truncate_http_URI = -1;

extern unsigned long long raw_get;
extern unsigned long long raw_post;
extern unsigned long long good_get;
extern unsigned long long good_post;

extern void pushPacket(std::string & request_header_host, std::string & url,
                std::string & title, std::string & login_msg, map<string, string> & mheaders,
                std::string & request_header_cookie, std::string & response_header_setcookie,
                const unsigned char * post_data, size_t post_data_size, std::string & source_ip,std::string & resp_ip, int response_code,
		std::string & request_timestamp, const char * request_method, const char * protocol );
extern void check_login_msg(std::string & host, const char * reply, size_t len, std::string & result);


/// list of supported HTTP methods
static const char* http_methods[] = {
		"GET", "POST", "HEAD",

		"OPTIONS", "PUT", "DELETE", "TRACE", "CONNECT",

		// HTTP methods for distributed authoring.
		"PROPFIND", "PROPPATCH", "MKCOL", /* "DELETE", "PUT", */
		"COPY", "MOVE", "LOCK", "UNLOCK",
		"POLL", "REPORT", "SUBSCRIBE", "BMOVE",

		"SEARCH",

		0,
	};

/**
 * Object constructor
 */
HTTP_Entity::HTTP_Entity(HTTP_Message *arg_message, MIME_Entity* parent_entity, int arg_expect_body)
:MIME_Entity(arg_message, parent_entity)
	{
	http_message = arg_message;
	parent = parent_entity;
	zip = NULL;
	Reset(arg_expect_body);
	}

/**
 * Object destructor.
 */
HTTP_Entity::~HTTP_Entity()
{
	if ( zip )
	{
		zip->Done();
		delete zip;
	}
}

/**
 * Reset() function is used when object is reused. All internal variables are changed to default values.
 */
void HTTP_Entity::Reset(int arg_expect_body)
{
	MIME_Entity::Reset();
	expect_body = arg_expect_body;
	deliver_body = (http_entity_data != 0);
	chunked_transfer_state = NON_CHUNKED_TRANSFER;
	chunked_content_length = 0;
	content_length = -1;	// unspecified
	expect_data_length = 0;
	body_length = 0;
	header_length = 0;
	encoding = IDENTITY;
	if ( zip )
	{
		zip->Done();
		delete zip; 
		zip = 0;
	}
}

/*
 * EndOfData() function is called when all request or response is received.
 */
void HTTP_Entity::EndOfData()
	{
	//if ( DEBUG_http )
	DEBUG_MSG("%.6f: end of data\n", network_time);

	if ( data_buf_offset != -1 && http_message->IsOrig() == false && 
		(content_type == CONTENT_TYPE_OTHER2 || content_type == CONTENT_TYPE_TEXT) &&
		(encoding == GZIP || encoding == DEFLATE ) && collect_reply == true)
	{
		zip = new ZIP_Analyzer(this);
		
		if ( zip )
			{
			int len = data_buf_offset;
			data_buf_offset = 0;
			body_length = 0;
			zip->DeliverStream(len, (const u_char*) data_buf_data, false);

			zip->Done();
			delete zip;
			zip = 0;
			encoding = IDENTITY;
			}
	}

	if ( data_buf_offset >= 10 && data_buf_data != NULL && 
		http_message->IsOrig() == false && collect_reply == true &&
		(content_type == CONTENT_TYPE_OTHER2 || content_type == CONTENT_TYPE_TEXT) )
	{
		http_message->CheckLoginMsg( data_buf_data, data_buf_offset );
		size_t title_len = 0;
		//std::string title;
		// look for HTML title
		char * pos = strcasestr(data_buf_data, "<title");
		char * end;
		if (pos > 0)
		{
			for (pos+= 6; *pos; pos++)
			{
				if (*pos == '>')
				{
					pos++;
					break;
				}
			}
			// skip spaces at the begining
			while (*pos == ' ' || *pos == '\t' || *pos == '\r' || *pos == '\n')
				pos++;
			for (end= pos; *end; end++)
			{
				if (*end == '<')
					break;
			}
			// skip spaces at the end
			while (end>pos && (*(end-1) == ' ' || *(end-1) == '\t' || *(end-1) == '\r' || *(end-1) == '\n'))
				end--;
			title_len = ((end-pos > 200 ) ? 200 : (end-pos));
			//title.assign(pos,title_len);
			//printf("title: %s\n", title.c_str());
			//title_len = 0;
		}
		if (title_len == 0)
		{
			// look for <h1>
			pos = strcasestr(data_buf_data, "<h1>");
			if (pos > 0)
			{
				pos += 4;
				// skip spaces at the begining
				while (*pos == ' ' || *pos == '\t' || *pos == '\r' || *pos == '\n')
					pos++;
				for (end= pos; *end; end++)
				{
					if (*end == '<')
						break;
				}
				// skip spaces at the end
				while (end>pos && (*(end-1) == ' ' || *(end-1) == '\t' || *(end-1) == '\r' || *(end-1) == '\n'))
					end--;
				title_len = ((end-pos > 200 ) ? 200 : (end-pos));
				//title.assign(pos,title_len);
				//printf("title from h1: %s\n", title.c_str());
			}
		}
		if (title_len != 0)
		{
			http_message->SubmitTitle(pos, title_len);
		}
	}

//	if ( body_length )
//		http_message->MyHTTP_Analyzer()->
//			ForwardEndOfData(http_message->IsOrig());

	MIME_Entity::EndOfData();
	}

/**
 * Deliver() function is called from ContentLine_Analyzer when new token (header line) is parsed or in case of plain delivery.
 */
void HTTP_Entity::Deliver(int len, const char* data, int trailing_CRLF)
	{
	if ( DEBUG_http )
		{
		DEBUG_MSG("%.6f HTTP_Entity::Deliver len=%d, in_header=%d\n",
		          network_time, len, in_header);
		}

	if ( end_of_data )
		{
		// Multipart entities may have trailers
		if ( content_type != CONTENT_TYPE_MULTIPART )
			IllegalFormat("data trailing the end of entity");
		return;
		}

	if ( in_header )
		{
		if ( ! trailing_CRLF )
			http_message->MyHTTP_Analyzer()->Weird("http_no_crlf_in_header_list");

		header_length += len;
		MIME_Entity::Deliver(len, data, trailing_CRLF);
		return;
		}

	// Entity body.
	if ( content_type == CONTENT_TYPE_MULTIPART ||
	     content_type == CONTENT_TYPE_MESSAGE )
		DeliverBody(len, data, trailing_CRLF);

	else if ( chunked_transfer_state != NON_CHUNKED_TRANSFER )
		{
		switch ( chunked_transfer_state ) {
		case EXPECT_CHUNK_SIZE:
			ASSERT(trailing_CRLF);
			if ( ! atoi_n(len, data, 0, 16, expect_data_length) )
				{
				http_message->Weird("HTTP_bad_chunk_size");
				expect_data_length = 0;
				}

			if ( expect_data_length > 0 )
				{
				chunked_content_length += expect_data_length;
				if (http_message->IsOrig() == true && chunked_content_length > max_post_size)
					{
					http_message->Clear();
					data_buf_offset = 0;
					}
				chunked_transfer_state = EXPECT_CHUNK_DATA;
				SetPlainDelivery(expect_data_length);
				}
			else
				{
				// This is the last chunk
				in_header = 1;
				chunked_transfer_state = EXPECT_CHUNK_TRAILER;
				}
			break;

		case EXPECT_CHUNK_DATA:
			ASSERT(! trailing_CRLF);
			ASSERT(len <= expect_data_length);
			expect_data_length -= len;
			if ( expect_data_length <= 0 )
				{
				SetPlainDelivery(0);
				chunked_transfer_state = EXPECT_CHUNK_DATA_CRLF;
				}
			// we will only process chunk request from client !
			if (http_message->IsOrig() == true)
				{
					DeliverBody(len, data, 0);
				}
			else if (collect_reply /* && chunked_content_length < max_post_size */)
				{
				DeliverBody(len, data, 0);
				}
			break;

		case EXPECT_CHUNK_DATA_CRLF:
			ASSERT(trailing_CRLF);
			if ( len > 0 )
				IllegalFormat("inaccurate chunk size: data before <CR><LF>");
			chunked_transfer_state = EXPECT_CHUNK_SIZE;
			break;
		}
		}

	else if ( content_length >= 0 )
		{
		ASSERT(! trailing_CRLF);
		ASSERT(len <= expect_data_length);

		if (http_message->IsOrig() == true )
			{
			DeliverBody(len, data, 0);
			}
		else if (collect_reply /* && content_length <= max_post_size */)
			{
			DeliverBody(len, data, 0);
			}

		expect_data_length -= len;
		if ( expect_data_length <= 0 )
			{
			SetPlainDelivery(0);
			EndOfData();
			}
		}

	else
		DeliverBody(len, data, trailing_CRLF);
	}

/*
class HTTP_Entity::UncompressedOutput : public Analyzer::OutputHandler {
public:
	UncompressedOutput(HTTP_Entity* e)	{ entity = e; }
	virtual	~UncompressedOutput() { }
	virtual void DeliverStream(int len, const u_char* data, bool orig)
		{
		entity->DeliverBodyClear(len, (char*) data, false);
		}
private:
	HTTP_Entity* entity;
};
*/

void HTTP_Entity::DeliverBody(int len, const char* data, int trailing_CRLF)
	{
		body_length += len;
		if ( trailing_CRLF )
			body_length += 2;

		if ( deliver_body )
			MIME_Entity::Deliver(len, data, trailing_CRLF);
	}

// Returns 1 if the undelivered bytes are completely within the body,
// otherwise returns 0.
int HTTP_Entity::Undelivered(int len)
	{
	if ( DEBUG_http )
		{
		DEBUG_MSG("Content gap %d, expect_data_length %d\n",
			  len, expect_data_length);
		}

	if ( end_of_data && in_header )
		return 0;

	if ( chunked_transfer_state != NON_CHUNKED_TRANSFER )
		{
		if ( chunked_transfer_state == EXPECT_CHUNK_DATA &&
		     expect_data_length >= len )
			{
			body_length += len;
			expect_data_length -= len;

			SetPlainDelivery(expect_data_length);
			if ( expect_data_length == 0 )
				chunked_transfer_state = EXPECT_CHUNK_DATA_CRLF;

			return 1;
			}
		else
			return 0;
		}

	else if ( content_length >= 0 )
		{
		if ( expect_data_length >= len )
			{
			body_length += len;
			expect_data_length -= len;

			SetPlainDelivery(expect_data_length);

			if ( expect_data_length <= 0 )
				EndOfData();

			return 1;
			}

		else
			return 0;
		}

	return 0;
	}

void HTTP_Entity::SubmitData(int len, const char* buf)
	{
	if ( deliver_body )
		MIME_Entity::SubmitData(len, buf);
	}

void HTTP_Entity::SetPlainDelivery(int64_t length)
	{
	ASSERT(length >= 0);
	ASSERT(length == 0 || ! in_header);

	http_message->SetPlainDelivery(length);

	// If we skip HTTP data, the skipped part will appear as
	// 'undelivered' data, so we do not need to adjust
	// expect_data_length.
	}

void HTTP_Entity::SubmitHeader(MIME_Header* h)
	{
	if ( strcasecmp_n(h->get_name(), "content-length") == 0 )
		{
		data_chunk_t vt = h->get_value_token();
		if ( ! is_null_data_chunk(vt) )
			{
			int64_t n;
			if ( atoi_n(vt.length, vt.data, 0, 10, n) )
				content_length = n;
			else
				content_length = 0;
			}
		}

	// Figure out content-length for HTTP 206 Partial Content response
	// that uses multipart/byteranges content-type.
	else if ( strcasecmp_n(h->get_name(), "content-range") == 0 && Parent() &&
	          Parent()->MIMEContentType() == CONTENT_TYPE_MULTIPART &&
		      http_message->MyHTTP_Analyzer()->HTTP_ReplyCode() == 206 )
		{
		data_chunk_t vt = h->get_value_token();
		string byte_unit(vt.data, vt.length);
		vt = h->get_value_after_token();
		string byte_range(vt.data, vt.length);
		byte_range.erase(remove(byte_range.begin(), byte_range.end(), ' '),
		                 byte_range.end());

		if ( byte_unit != "bytes" )
			{
			http_message->Weird("HTTP_content_range_unknown_byte_unit");
			return;
			}

		size_t p = byte_range.find("/");
		if ( p == string::npos )
			{
			http_message->Weird("HTTP_content_range_cannot_parse");
			return;
			}

		string byte_range_resp_spec = byte_range.substr(0, p);
		string instance_length = byte_range.substr(p + 1);

		p = byte_range_resp_spec.find("-");
		if ( p == string::npos )
			{
			http_message->Weird("HTTP_content_range_cannot_parse");
			return;
			}

		string first_byte_pos = byte_range_resp_spec.substr(0, p);
		string last_byte_pos = byte_range_resp_spec.substr(p + 1);

		if ( DEBUG_http )
			DEBUG_MSG("Parsed Content-Range: %s %s-%s/%s\n", byte_unit.c_str(),
		              first_byte_pos.c_str(), last_byte_pos.c_str(),
		              instance_length.c_str());

		int64_t f, l;
		atoi_n(first_byte_pos.size(), first_byte_pos.c_str(), 0, 10, f);
		atoi_n(last_byte_pos.size(), last_byte_pos.c_str(), 0, 10, l);
		int64_t len = l - f + 1;

		//if ( DEBUG_http )
		DEBUG_MSG("Content-Range length = %"PRId64"\n", len);

		if ( len > 0 )
			content_length = len;
		else
			{
			http_message->Weird("HTTP_non_positive_content_range");
			return;
			}
		}

	else if ( strcasecmp_n(h->get_name(), "transfer-encoding") == 0 )
		{
		data_chunk_t vt = h->get_value_token();
		if ( strcasecmp_n(vt, "chunked") == 0 )
			{
			chunked_transfer_state = BEFORE_CHUNK;
			}
		}

	else if ( strcasecmp_n(h->get_name(), "content-encoding") == 0 )
		{
		data_chunk_t vt = h->get_value_token();
		if ( strcasecmp_n(vt, "gzip") == 0 )
			encoding = GZIP;
		if ( strcasecmp_n(vt, "deflate") == 0 )
			encoding = DEFLATE;
		}

	MIME_Entity::SubmitHeader(h);
	}

int HTTP_Entity::SubmitAllHeaders()
	{
	// in_header should be set to false when SubmitAllHeaders() is called.
	ASSERT(! in_header);

	if ( DEBUG_http )
		DEBUG_MSG("%.6f end of headers\n", network_time);

	// The presence of a message-body in a request is signaled by
	// the inclusion of a Content-Length or Transfer-Encoding
	// header field in the request's message-headers.
	if ( chunked_transfer_state == EXPECT_CHUNK_TRAILER )
		{
		http_message->SubmitTrailingHeaders();
		chunked_transfer_state = EXPECT_NOTHING;
		EndOfData();
		return HTTP_BODY_NOT_EXPECTED;
		}

	MIME_Entity::SubmitAllHeaders();

	if ( expect_body == HTTP_BODY_NOT_EXPECTED )
		{
		EndOfData();
		return HTTP_BODY_NOT_EXPECTED;
		}

	if ( content_type == CONTENT_TYPE_MULTIPART ||
	     content_type == CONTENT_TYPE_MESSAGE )
		{
		// Do nothing.
		// Make sure that we check for multiple/message contents first,
		// because we do not have to turn on .
		if ( chunked_transfer_state != NON_CHUNKED_TRANSFER )
			{
			http_message->Weird(
				"HTTP_chunked_transfer_for_multipart_message");
			}
		}

	else if ( chunked_transfer_state != NON_CHUNKED_TRANSFER )
		chunked_transfer_state = EXPECT_CHUNK_SIZE;

	else if ( content_length >= 0 )
		{
		if ( content_length > 0 )
			{
			expect_data_length = content_length;
			SetPlainDelivery(content_length);
			}
		else
			{
			EndOfData(); // handle the case that content-length = 0
			return HTTP_BODY_NOT_EXPECTED;
			}
		}

	// Turn plain delivery on permanently for compressed bodies without
	// content-length headers or if connection is to be closed afterwards
	// anyway.
	else if ( http_message->MyHTTP_Analyzer()->IsConnectionClose ()
		  || encoding == GZIP || encoding == DEFLATE
		 )
		{
		// FIXME: Using INT_MAX is kind of a hack here.  Better
		// would be to make -1 as special value interpreted as
		// "until the end of the connection".
		expect_data_length = INT_MAX;
		SetPlainDelivery(INT_MAX);
		}

	else
		{
		if ( expect_body != HTTP_BODY_EXPECTED )
			{
			// there is no body
			EndOfData();
			return HTTP_BODY_NOT_EXPECTED;
			}
		}
	return HTTP_BODY_MAYBE;
	}

HTTP_Message::HTTP_Message(HTTP_Analyzer* arg_analyzer,
				ContentLine_Analyzer* arg_cl, bool arg_is_orig,
				int expect_body, int64_t init_header_length)
//: MIME_Message (/* arg_analyzer */)
: top_level(this, 0, expect_body)
	{
	analyzer = arg_analyzer;
	content_line = arg_cl;
	is_orig = arg_is_orig;

	current_entity = 0;
	//top_level = new HTTP_Entity(this, 0, expect_body);
	BeginEntity(&top_level);

	//data_buffer = 0;
	buffer_size = max_post_size;
	buffer_offset = 0;
	total_buffer_size = max_post_size;
	finished = 0;
	active = false;

	start_time = network_time;
	body_length = 0;
	content_gap_length = 0;
	header_length = init_header_length;
	// try to fix valgrind warning
	//memset(data_buffer, 0, sizeof(data_buffer));
	data_buffer[sizeof(data_buffer)-1] = '\0';
	}

HTTP_Message::~HTTP_Message()
	{
	//if (top_level)
	//	{
	//	delete top_level;
	//	top_level = NULL;
	//	}

	/*
	delete_strings(buffers);
	if ( data_buffer )
		{
			delete data_buffer;
			data_buffer = 0;
		}
	*/
	ClearHeaders();
	}

void HTTP_Message::Reset(int expect_body, int64_t init_header_length, bool active_arg)
{
	active = active_arg;
	top_level.Reset(expect_body);
	//top_level = new HTTP_Entity(this, 0, expect_body);
	BeginEntity(&top_level);

	/*
	delete_strings(buffers);
	if ( data_buffer )
	{
		delete data_buffer;
		data_buffer = 0;
	}
	*/
	buffer_size = max_post_size;
	buffer_offset = 0;
	total_buffer_size = max_post_size;
	finished = 0;
	data_buffer[0] = '\0';

	start_time = network_time;
	body_length = 0;
	content_gap_length = 0;
	header_length = init_header_length;
	ClearHeaders();
}

void HTTP_Message::ClearHeaders()
{
	headers.clear();
}

void HTTP_Message::Clear()
	{
	/*
	delete_strings(buffers);
	if ( data_buffer )
	{
		delete data_buffer;
		data_buffer = 0;
	}
	*/
	body_length = 0;
	buffer_size = max_post_size;
	buffer_offset = 0;
	total_buffer_size = max_post_size;
	}

void HTTP_Message::Done(const int interrupted, const char* detail)
	{
	if ( finished )
		return;

	// set finished = 1
	//MIME_Message::Done();
	finished = 1;
	active = false;

	// DEBUG_MSG("%.6f HTTP message done.\n", network_time);
	top_level.EndOfData();

	/*
	delete_strings(buffers);
	if ( data_buffer )
		{
		delete data_buffer;
		data_buffer = 0;
		}
	*/

	//CHECK THIS OUT XXXXXX
	MyHTTP_Analyzer()->HTTP_MessageDone(is_orig, this);

	}

void HTTP_Message::Deliver(int len, const char* data, int trailing_CRLF)
	{
	//if (top_level)
		top_level.Deliver(len, data, trailing_CRLF);
	}

int HTTP_Message::Undelivered(int len)
	{
	//if ( ! top_level )
	//	return 0;

	if ( ((HTTP_Entity*) &top_level)->Undelivered(len) )
		{
		content_gap_length += len;
		return 1;
		}

	return 0;
	}

void HTTP_Message::BeginEntity(MIME_Entity* entity)
	{
	if ( DEBUG_http )
		DEBUG_MSG("%.6f: begin entity (%d)\n", network_time, is_orig);

	current_entity = (HTTP_Entity*) entity;
	}

void HTTP_Message::EndEntity(MIME_Entity* entity)
	{
	if ( DEBUG_http )
		DEBUG_MSG("%.6f: end entity (%d)\n", network_time, is_orig);

	body_length += ((HTTP_Entity*) entity)->BodyLength();
	header_length += ((HTTP_Entity*) entity)->HeaderLength();

	DeliverEntityData();

	current_entity = (HTTP_Entity*) entity->Parent();

	// It is necessary to call Done when EndEntity is triggered by
	// SubmitAllHeaders (through EndOfData).
	if ( entity == &top_level )
	{
		Done();
		if (is_orig == false)
		{
			MyHTTP_Analyzer()->Dump();
		}
	}
	}

void HTTP_Message::SubmitHeader(MIME_Header* h)
	{
	// we only collect request headers !
	if (is_orig == true)
	{
		temp_key.assign(h->get_name().data, h->get_name().length);
		transform(temp_key.begin(),temp_key.end(),temp_key.begin(),::tolower);
		//transform(h->get_name().data,h->get_name().data + h->get_name().length, key.begin(),::tolower);
		temp_value.assign(h->get_value().data, h->get_value().length);
		//headers.push_back(h);
		headers[temp_key] = temp_value;
	}
	MyHTTP_Analyzer()->HTTP_Header(is_orig, h);
	}

int HTTP_Message::SubmitAllHeaders()
	{
	int ret = MyHTTP_Analyzer()->SubmitAllHeaders(is_orig);
	return ret;
	}

void HTTP_Message::SubmitTrailingHeaders()
	{
	// Do nothing for now.
		DEBUG_MSG("got response, submitting headers\n");
	}

void HTTP_Message::SubmitData(int len, const char* buf)
	{
	if ( buf != (const char*) data_buffer + buffer_offset ||
	     buffer_offset + len > buffer_size )
		reporter->InternalError("buffer misalignment");

	buffer_offset += len;
	/*
	if ( buffer_offset >= buffer_size )
		{
		buffers.push_back(data_buffer);
		data_buffer = 0;
		}
	*/
	}

int HTTP_Message::RequestBuffer(int* plen, char** pbuf)
	{
	if ( ! http_entity_data )
		return 0;

	InitBuffer(mime_segment_length);
	//if ( ! data_buffer )
	//	if ( ! InitBuffer(mime_segment_length) )
	//		return 0;

	//*plen = data_buffer->size() - buffer_offset;
	*plen = buffer_size - buffer_offset;
	*pbuf = (char*) data_buffer + buffer_offset;

	return 1;
	}

void HTTP_Message::SubmitAllData()
	{
	// This marks the end of message
		DEBUG_MSG("submit all data\n");
	}

void HTTP_Message::SubmitEvent(int event_type, const char* detail)
	{
/*
	const char* category = "";

	switch ( event_type ) {
	case MIME_EVENT_ILLEGAL_FORMAT:
		category = "illegal format";
		break;

	case MIME_EVENT_ILLEGAL_ENCODING:
		category = "illegal encoding";
		break;

	case MIME_EVENT_CONTENT_GAP:
		category = "content gap";
		break;

	default:
		reporter->InternalError("unrecognized HTTP message event");
	}
*/

//	MyHTTP_Analyzer()->HTTP_Event(category, detail);
	}

void HTTP_Message::SubmitTitle(const char * data, size_t len)
{
	analyzer->SubmitTitle(data, len);
}

void HTTP_Message::CheckLoginMsg(const char * reply, size_t len)
{
	analyzer->CheckLoginMsg(reply, len);
}

void HTTP_Message::SetPlainDelivery(int64_t length)
	{
	content_line->SetPlainDelivery(length);

	if ( length > 0 && BifConst::skip_http_data )
		content_line->SkipBytesAfterThisLine(length);
	else if ( ! data_buffer )
		InitBuffer(length);
	}

void HTTP_Message::SetContentType(int c_type)
{
	top_level.SetContentType(c_type);
}

void HTTP_Message::SetCollectReply(bool value)
{
	top_level.SetCollectReply(value);
}

void HTTP_Message::SkipEntityData()
	{
	if ( current_entity )
		current_entity->SkipBody();
	}

void HTTP_Message::DeliverEntityData()
	{
	if ( http_entity_data && buffer_offset && content_gap_length == 0 )
		{
		MyHTTP_Analyzer()->HTTP_EntityData(is_orig, data_buffer, buffer_offset);
		}
	else if ( content_gap_length != 0 )
		{
		DEBUG_MSG("we have a content gap, post or response will not be forwarded for processing\n");
		}
	buffer_offset = 0;
	}

int HTTP_Message::InitBuffer(int64_t length)
	{
	if ( length <= 0 )
		return 0;

	//if (buffer_offset == http_entity_data_delivery_size)
	//	DeliverEntityData();

	//if ( total_buffer_size >= http_entity_data_delivery_size )
	//	DeliverEntityData();

	//if ( total_buffer_size + length > http_entity_data_delivery_size )
	//	{
	//	length = http_entity_data_delivery_size - total_buffer_size;
	//	if ( length <= 0 )
	//		return 0;
	//	}

	//data_buffer = new std::string();
	//data_buffer->resize(length, 0);

	//buffer_size = length;
	//total_buffer_size += length;
	total_buffer_size = buffer_size;
	buffer_offset = 0;

	return 1;
	}

void HTTP_Message::Weird(const char* msg)
	{
	analyzer->Weird(msg);
	}

HTTP_Analyzer::HTTP_Analyzer(TCP_Analyzer* _tcp, ContentLine_Analyzer* content_line_orig, ContentLine_Analyzer* content_line_resp)
: request_message(this, content_line_orig, true), reply_message(this, content_line_resp, false)
/*	: TCP_ApplicationAnalyzer(AnalyzerTag::HTTP, conn) */
	{
	tcp = _tcp;
	num_requests = num_replies = 0;
	num_request_lines = num_reply_lines = 0;
	request_version = reply_version = 0.0;	// unknown version
	keep_alive = 0;
	connection_close = 0;
	unanswered_requests = NULL;

	//request_message = reply_message = 0;
	//request_headers = reply_headers = 0;
	request_state = EXPECT_REQUEST_LINE;
	reply_state = EXPECT_REPLY_LINE;

	current_post = NULL;
	current_post_size = 0;
	request_ongoing = 0;
	//request_method = request_URI = 0;
	//unescaped_URI = 0;

	reply_ongoing = 0;
	reply_code = 0;

	}

HTTP_Analyzer::~HTTP_Analyzer()
	{
	// make sure that request is closed
	Done();
	request_message.ClearHeaders();
	reply_message.ClearHeaders();
	//if (request_method)
	//	delete request_method;
	//if (request_URI)
	//	delete request_URI;
	//if (unescaped_URI)
	//	delete unescaped_URI;
	//if (request_message)
	//{
	//	delete request_message;
	//}
	//if (reply_message)
	//{
	//	delete reply_message;
	//}
	//if (current_post)
	//{
	//	delete current_post;
	//}
	//reply_reason_phrase.clear();
/*
	Unref(request_method);
	Unref(request_URI);
	Unref(unescaped_URI);
	Unref(reply_reason_phrase);
*/
	}

void HTTP_Analyzer::Reset()
{
	num_requests = num_replies = 0;
	num_request_lines = num_reply_lines = 0;
	request_version = reply_version = 0.0;	// unknown version
	keep_alive = 0;
	connection_close = 0;
	unanswered_requests = NULL;
	request_state = EXPECT_REQUEST_LINE;
	reply_state = EXPECT_REPLY_LINE;

	//if (current_post)
	//{
	//	delete current_post;
	//}

	current_post = NULL;
	current_post_size = 0;
	request_ongoing = 0;

	reply_ongoing = 0;
	reply_code = 0;

	request_method = NULL;
	request_URI.clear();
	//unescaped_URI.clear();
	title.clear();
	login_msg.clear();

	reply_message.Reset(0,0);
	request_message.Reset(0,0);
}

void HTTP_Analyzer::Done()
	{
	Dump();
/*
	if ( IsFinished() )
		return;
*/

//	TCP_ApplicationAnalyzer::Done();

	RequestMade(1, "message interrupted when connection done");
	ReplyMade(1, "message interrupted when connection done");

	//if (request_message)
	//	{
	//	delete request_message;
	//	request_message = 0;
	//	}

	//if (reply_message)
	//	{
	//	delete reply_message;
	//	reply_message = 0;
	//	}
	
	GenStats();

	/*
	while ( ! unanswered_requests.empty() )
		{
		//Unref(unanswered_requests.front());
		//delete unanswered_requests.front();
		unanswered_requests.pop();
		}
	*/
	unanswered_requests = NULL;
	}

void HTTP_Analyzer::Weird(const char * msg)
        {
                tcp->Weird(msg);
        }

void HTTP_Analyzer::ProtocolViolation(const char * msg)
        {
                //DEBUG_MSG(msg);
                tcp->Weird(msg);
        }


void HTTP_Analyzer::DeliverStream(int len, const u_char* data, bool is_orig, ContentLine_Analyzer* content_line)
	{
//	TCP_ApplicationAnalyzer::DeliverStream(len, data, is_orig);

	if ( tcp && tcp->IsPartial() )
	{
		DEBUG_MSG("partial tcp !\n");
		//return;
	}

	const char* line = reinterpret_cast<const char*>(data);
	const char* end_of_line = line + len;

//	ContentLine_Analyzer* content_line =
//		is_orig ? content_line_orig : content_line_resp;

	if ( content_line->IsPlainDelivery() )
		{
		if ( is_orig )
			{
			if ( request_message.Active() )
				request_message.Deliver(len, line, 0);
			else
				Weird("unexpected_client_HTTP_data");
			}
		else
			{
			if ( reply_message.Active() )
				reply_message.Deliver(len, line, 0);
			else
				Weird("unexpected_server_HTTP_data");
			}
		return;
		}

	// HTTP_Event("HTTP line", new_string_val(length, line));

	if ( is_orig )
		{
		++num_request_lines;

		switch ( request_state ) {
		case EXPECT_REQUEST_LINE:
			DEBUG_MSG("request line: %s\n", data);
			if ( HTTP_RequestLine(line, end_of_line) )
				{
				++num_requests;

				if ( ! keep_alive && num_requests > 1 )
					Weird("unexpected_multiple_HTTP_requests");

				request_state = EXPECT_REQUEST_MESSAGE;
				request_ongoing = 1;
				//unanswered_requests.push(request_method);
				unanswered_requests = request_method;
				HTTP_Request();
				InitHTTPMessage(is_orig, HTTP_BODY_MAYBE, len);
				// check if previous response was broken, if yes try to read new lineS
				if (reply_state == EXPECT_REPLY_NOTHING)
					reply_state = EXPECT_REPLY_LINE;
				}

			else
				{
				if ( ! RequestExpected() )
					;
//					HTTP_Event("crud_trailing_HTTP_request", new_string_val(line, end_of_line));
				else
					{
					// We do see HTTP requests with a
					// trailing EOL that's not accounted
					// for by the content-length. This
					// will lead to a call to this method
					// with len==0 while we are expecting
					// a new request. Since HTTP servers
					// handle such requests gracefully,
					// we should do so as well.
					if ( len == 0 )
					    Weird("empty_http_request");
					else
						{
						ProtocolViolation("not a http request line");
						request_state = EXPECT_REQUEST_NOTHING;
						}
					}
				}
			break;

		case EXPECT_REQUEST_MESSAGE:
			//DEBUG_MSG("request message: %s\n", data);
			request_message.Deliver(len, line, 1);
			break;

		case EXPECT_REQUEST_TRAILER:
			break;

		case EXPECT_REQUEST_NOTHING:
			break;
		}
		}
	else
		{ // HTTP reply
		switch ( reply_state ) {
		case EXPECT_REPLY_LINE:
			// make sure to reset all collected response data in case we had something broken before
			title.clear();
			login_msg.clear();
			reply_code = 0;
			reply_reason_phrase.clear();
			set_cookie.clear();
			DEBUG_MSG("reply line: %s\n", data);
			if ( HTTP_ReplyLine(line, end_of_line) )
				{
				++num_replies;

				//if ( unanswered_requests.empty() )
				if (unanswered_requests == NULL)
				{
					//Weird("unmatched_HTTP_reply");
					DEBUG_MSG("unmatched_HTTP_reply\n");
					if (reply_code != 100)
					{
						request_state = EXPECT_REQUEST_RESET;
					}
				}
				else
					ProtocolConfirmation();

				reply_state = EXPECT_REPLY_MESSAGE;
				reply_ongoing = 1;

				HTTP_Reply();

				InitHTTPMessage(is_orig, ExpectReplyMessageBody(), len);
				}
			else
				{
				ProtocolViolation("not a http reply line");
				reply_state = EXPECT_REPLY_NOTHING;
				}

			break;

		case EXPECT_REPLY_MESSAGE:
			//DEBUG_MSG("reply line: %s\n", data);
			reply_message.Deliver(len, line, 1);
			break;

		case EXPECT_REPLY_TRAILER:
			break;

		case EXPECT_REPLY_NOTHING:
			break;
		}
		}
	}

void HTTP_Analyzer::Undelivered(int seq, int len, bool is_orig, ContentLine_Analyzer* content_line)
	{
//	TCP_ApplicationAnalyzer::Undelivered(seq, len, is_orig);

	if (is_orig)
		{
		DEBUG_MSG("Undelivered request from %d: %d bytes\n", seq, len);
		}
	else
		{
		DEBUG_MSG("Undelivered reply from %d: %d bytes\n", seq, len);
		}

	HTTP_Message* msg =
		is_orig ? &request_message : &reply_message;

//	ContentLine_Analyzer* content_line =
//		is_orig ? content_line_orig : content_line_resp;

	if ( ! content_line->IsSkippedContents(seq, len) )
		{
		if ( msg )
			msg->SubmitEvent(MIME_EVENT_CONTENT_GAP,
				fmt("seq=%d, len=%d", seq, len));
		}

	// Check if the content gap falls completely within a message body
	if ( msg && msg->Undelivered(len) )
		// If so, we are safe to skip the content and go on parsing
		return;

	// Otherwise stop parsing the connection
	if ( is_orig )
		{
		// Stop parsing reply messages too, because whether a
		// reply contains a body may depend on knowing the
		// request method

		RequestMade(1, "message interrupted by a content gap");
		ReplyMade(1, "message interrupted by a content gap");

		//content_line->SetSkipDeliveries(1);
		//content_line->SetSkipDeliveries(1);
		}
	else
		{
		ReplyMade(1, "message interrupted by a content gap");
		//content_line->SetSkipDeliveries(1);
		}
	}

void HTTP_Analyzer::EndpointEOF(bool is_orig)
	{
//	TCP_ApplicationAnalyzer::EndpointEOF(is_orig);

	// DEBUG_MSG("%.6f eof\n", network_time);

	if ( is_orig )
		RequestMade(0, "message ends as connection contents are completely delivered");
	else
		ReplyMade(0, "message ends as connection contents are completely delivered");
	}

void HTTP_Analyzer::ConnectionFinished(int half_finished)
	{
//	TCP_ApplicationAnalyzer::ConnectionFinished(half_finished);

	// DEBUG_MSG("%.6f connection finished\n", network_time);
	RequestMade(1, "message ends as connection is finished");
	ReplyMade(1, "message ends as connection is finished");
	}

void HTTP_Analyzer::ConnectionReset()
	{
//	TCP_ApplicationAnalyzer::ConnectionReset();

	RequestMade(1, "message interrupted by RST");
	ReplyMade(1, "message interrupted by RST");
	}

/**
 * PacketWithRST() function is called when TCP RST packet is received.
 */
void HTTP_Analyzer::PacketWithRST()
	{
//	TCP_ApplicationAnalyzer::PacketWithRST();

	RequestMade(1, "message interrupted by RST");
	ReplyMade(1, "message interrupted by RST");
	}

void HTTP_Analyzer::GenStats()
	{
	}

/**
 * PrefixMatch() function is used to search substring at the begining of a string
 */
const char* HTTP_Analyzer::PrefixMatch(const char* line,
				const char* end_of_line, const char* prefix)
	{
	while ( *prefix && line < end_of_line && *prefix == *line )
		{
		++prefix;
		++line;
		}

	if ( *prefix )
		// It didn't match.
		return 0;

	return line;
	}

const char* HTTP_Analyzer::PrefixWordMatch(const char* line,
				const char* end_of_line, const char* prefix)
	{
	if ( (line = PrefixMatch(line, end_of_line, prefix)) == 0 )
		return 0;

	const char* orig_line = line;
	line = skip_whitespace(line, end_of_line);

	if ( line == orig_line )
		// Word didn't end at prefix.
		return 0;

	return line;
	}

/**
 * Dump() is a main function of our BRO hack. It returns back to us distict requests and responses.
 */
void HTTP_Analyzer::Dump()
{
	static const char * protocol_https = "HTTPS";
	const char * protocol = "HTTP";
	char d[20];

	//if (request_message == NULL || reply_message == NULL)
	//	return;
	//if (request_message.headers.size() == 0 || reply_message.headers.size() == 0)
	//	return;
	if (request_URI.empty())
		return;
	if (reply_code == 0)
		return;
	// we failed to parse this request method !
	if (request_method == 0)
		return;
	// skip next header
	// HTTP/1.1 100 Continue
	if (reply_code >= 100 && reply_code < 200)
	{
		return;
	}
	if (tcp->IsEncrypted())
	{
		protocol = protocol_https;
	} else {
		if (request_method == http_methods[0])
		{
			// GET / request
			good_get++;
		} else if (request_method == http_methods[1])
		{
			// POST / request
			good_post++;
		}
	}

	DEBUG_MSG("===========================================\n");
	DEBUG_MSG("Dump[%s]: %s %s\n", protocol, request_method, request_URI.c_str());
	DEBUG_MSG("Reply: %d - %s\n", reply_code, reply_reason_phrase.c_str());
	if (title.size())
	{
		DEBUG_MSG("TITLE: %s\n", title.c_str());
	}
	if (login_msg.size())
	{
		DEBUG_MSG("LOGIN CONFIRMATION FOUND: %s\n", login_msg.c_str());
	}
	if (!set_cookie.empty())
	{
		DEBUG_MSG("Set-Cookie: %s\n", set_cookie.c_str());
	}

        std::string host;
        std::string cookie;
        std::string source_ip;
	std::string resp_ip;
        std::string timestamp;
	//static std::string empty;

	source_ip = tcp->OrigAddr().AsURIString();
	resp_ip = tcp->RespAddr().AsURIString();

	sprintf(d, "%.6f", tcp->LastTime());
	timestamp = d;

	DEBUG_MSG("Request:\n");
	std::map< std::string, std::string >::iterator itr;
	itr = request_message.headers.find("host");
	if (itr != request_message.headers.end())
	{
		host = itr->second;
		transform(host.begin(),host.end(),host.begin(),::tolower);
		if (host.find("www.") == 0)
			host.erase(0,4);
	}
	itr = request_message.headers.find("cookie");
	if (itr != request_message.headers.end())
	{
		cookie = itr->second;
	}
#ifdef BRO_DEBUG
	{
		for (itr = request_message.headers.begin(); itr != request_message.headers.end(); itr++)
		{
			DEBUG_MSG("H: %s => %s\n", itr->first.c_str(), itr->second.c_str());
		}
	}
#endif

	/*

	// we do not need to parse reply headers here
	// we just need set-cookie header !!!

	for (i = 0 ; i< nheaders; i++)
	{
		h = reply_message.headers[i];
		len = h->get_name().length;
		if (len == 0)
			continue;
		DEBUG_MSG("[" SIZET_FMT "] %s\n", len, h->get_name().data);
		if (len != 10)
		{
			continue;
		}
		if (strncmp(h->get_name().data, "Set-Cookie",10) == 0)
		{
			if (!set_cookie.empty())
				set_cookie.append("; ");
			set_cookie.append(h->get_value().data, h->get_value().length);
		}
	}
	reply_message.ClearHeaders();
	*/
#ifndef WIN32
	if (current_post_size)
	{
		DEBUG_MSG("------------------------------------------\n");
		DEBUG_MSG("req:\n%s\n", (const char*)current_post);
		pushPacket(host, request_URI, title, login_msg, request_message.headers, cookie, set_cookie, current_post, current_post_size, source_ip , resp_ip , reply_code, timestamp, request_method, protocol);
	} else {
		pushPacket(host, request_URI, title, login_msg, request_message.headers, cookie, set_cookie, (const unsigned char*)"", 0, source_ip, resp_ip , reply_code, timestamp, request_method, protocol);
	}
#endif

	request_message.ClearHeaders();
	current_post = NULL;
	current_post_size = 0;
	//unescaped_URI.clear();
	title.clear();
	login_msg.clear();
	reply_code = 0;
	reply_reason_phrase.clear();
	set_cookie.clear();
	tcp->GotReply();
}

/**
 * Save page title for later use.
 */
void HTTP_Analyzer::SubmitTitle(const char * data, size_t len)
{
	title.assign(data, len);
	if (reply_code == 200 && title == "404 - File or directory not found.")
	{
		reply_code = 404;
	}
}

/**
 * CheckLoginMsg() function is used to search substring in response. It is used to detect confirmation message after user logs-in using form based authentication.
 */
void HTTP_Analyzer::CheckLoginMsg( const char * reply, size_t len)
{
	std::string host;
	std::map< std::string, std::string >::iterator itr;
	itr = request_message.headers.find("host");
	if (itr == request_message.headers.end())
		return;
	host = itr->second;
	transform(host.begin(),host.end(),host.begin(),::tolower);
	if (host.find("www.") == 0)
		host.erase(0,4);
#ifndef WIN32
	check_login_msg( host, reply, len, login_msg );
#endif
}

/**
 * HTTP_RequestLine() function parses request line. It looks for request method. For example GET / POST / HEAD .
 */
int HTTP_Analyzer::HTTP_RequestLine(const char* line, const char* end_of_line)
	{
	const char* rest = 0;
	int i;
	for ( i = 0; http_methods[i]; ++i )
		if ( (rest = PrefixWordMatch(line, end_of_line, http_methods[i])) != 0 )
			break;

	if ( ! http_methods[i] )
		{
		DEBUG_MSG("HTTP_unknown_method: %s\n", line);
		if ( RequestExpected() )
			{	
			//			HTTP_Event("unknown_HTTP_method", new_string_val(line, end_of_line));
			}
		request_method = NULL;
		return 0;
		}

	request_method = http_methods[i];

	if ( ! ParseRequest(rest, end_of_line) )
		reporter->InternalError("HTTP ParseRequest failed");

//	Conn()->Match(Rule::HTTP_REQUEST,
//			(const u_char*) unescaped_URI->AsString()->Bytes(),
//			unescaped_URI->AsString()->Len(), true, true, true, true);

	return 1;
	}

/**
 * ParseRequest() function extracts request url and HTTP version out ot request line.
 */
int HTTP_Analyzer::ParseRequest(const char* line, const char* end_of_line)
	{
	const char* end_of_uri;
	const char* version_start;
	const char* version_end;

	for ( end_of_uri = line; end_of_uri < end_of_line; ++end_of_uri )
		{
		if ( ! is_reserved_URI_char(*end_of_uri) &&
		     ! is_unreserved_URI_char(*end_of_uri) &&
		     *end_of_uri != '%' )
			break;
		}

	for ( version_start = end_of_uri; version_start < end_of_line; ++version_start )
		{
		end_of_uri = version_start;
		version_start = skip_whitespace(version_start, end_of_line);
		if ( PrefixMatch(version_start, end_of_line, "HTTP/") )
			break;
		}

	if ( version_start >= end_of_line )
		{
		// If no version is found
		// init fake reply header !
		// because we have no headers !
		reply_state = EXPECT_REPLY_MESSAGE;
		reply_ongoing = 1;
		HTTP_Reply();
		InitHTTPMessage(false, true, 0);
		reply_code = 200;
		reply_message.SetPlainDelivery(INT_MAX);
		reply_message.Deliver(0, "", 1);
		reply_message.SetContentType( CONTENT_TYPE_TEXT );
		reply_message.SetCollectReply( true );
		SetVersion(request_version, 0.9);
		}
	else
		{
		if ( version_start + 8 <= end_of_line )
			{
			version_start += 5; // "HTTP/"
			SetVersion(request_version,
					HTTP_Version(end_of_line - version_start,
							version_start));

			version_end = version_start + 3;
			if ( skip_whitespace(version_end, end_of_line) != end_of_line )
				{
				DEBUG_MSG("crud after HTTP version is ignored\n");
				}
			}
		else
			{
				DEBUG_MSG("bad_HTTP_version: %s\n", line);
			}
		}

	// NormalizeURI(line, end_of_uri);

	request_URI.assign(line, end_of_uri - line);
	//unescape_URI((const u_char*) line, (const u_char*) end_of_uri, (TCP_Analyzer*)this, unescaped_URI);

	return 1;
	}

/**
 * Parse HTTP version string. Only recognize [0-9][.][0-9].
 */
double HTTP_Analyzer::HTTP_Version(int len, const char* data)
	{
	if ( len >= 3 &&
	     data[0] >= '0' && data[0] <= '9' &&
	     data[1] == '.' &&
	     data[2] >= '0' && data[2] <= '9' )
		{
		return double(data[0] - '0') + 0.1 * double(data[2] - '0');
		}
	else
		{
//        HTTP_Event("bad_HTTP_version", new_string_val(len, data));
		return 0;
		}
	}

/**
 * Change version number.
 */
void HTTP_Analyzer::SetVersion(double& version, double new_version)
	{
	if ( version == 0.0 )
	{
		version = new_version;
	}
	else if ( version != new_version )
	{
		//fprintf(stdout, "HTTP_version_mismatch: %f , %f\n", version, new_version);
		//Weird("HTTP_version_mismatch");
		version = new_version;
	}

	if ( version > 1.05 )
		keep_alive = 1;
	}

/**
 * TruncateURI() function truncates too long url.
 */
std::string * HTTP_Analyzer::TruncateURI(std::string* uri)
	{
	if ( truncate_http_URI >= 0 && uri->size() > truncate_http_URI )
		{
			uri->erase(truncate_http_URI);
			uri->append("...");
		}
		return uri;
	}

void HTTP_Analyzer::HTTP_Request()
	{
	ProtocolConfirmation();
	}

void HTTP_Analyzer::HTTP_Reply()
	{
	}

void HTTP_Analyzer::RequestMade(const int interrupted, const char* msg)
	{
	if ( ! request_ongoing )
		return;

	request_ongoing = 0;

//	if ( request_message )
		{
		request_message.Done(interrupted, msg);
		// XXX
		//delete request_message;
		//request_message = 0;
		DEBUG_MSG("request_message received !\n");
		}

	// DEBUG_MSG("%.6f request made\n", network_time);

//	Unref(request_method);
//	Unref(unescaped_URI);
//	Unref(request_URI);
	//if (unescaped_URI)
	//	delete unescaped_URI;
	//if (request_URI)
	//	delete request_URI;

	//request_method = request_URI = unescaped_URI = 0;

	num_request_lines = 0;

	if ( interrupted )
		request_state = EXPECT_REQUEST_NOTHING;
	else
		request_state = EXPECT_REQUEST_LINE;
	}

void HTTP_Analyzer::ReplyMade(const int interrupted, const char* msg)
	{
	if ( ! reply_ongoing )
		return;

	reply_ongoing = 0;

	// DEBUG_MSG("%.6f reply made\n", network_time);

	if ( reply_message.Active() )
		reply_message.Done(interrupted, msg);

	// 1xx replies do not indicate the final response to a request,
	// so don't pop an unanswered request in that case.
	if ( (reply_code < 100 || reply_code >= 200) && unanswered_requests )
		{
		// we can remove this request !
		// we got a response
		//Unref(unanswered_requests.front());
		//delete unanswered_requests.front();
		//unanswered_requests.pop();
		unanswered_requests = NULL;
		}
	else
		{
			if ( unanswered_requests )
				{
				DEBUG_MSG("we need to waite for another http response, got %d\n", reply_code);
				}
		}

	//reply_code = 0;
	//reply_reason_phrase.clear();

	if ( interrupted )
		reply_state = EXPECT_REPLY_NOTHING;
	else
		reply_state = EXPECT_REPLY_LINE;
	}

/*
void HTTP_Analyzer::RequestClash(Val* clash_val)
	{
	Weird("multiple_HTTP_request_elements");

	// Flush out old values.
	RequestMade(1, "request clash");
	}
*/

const char * HTTP_Analyzer::UnansweredRequestMethod()
	{
	//return unanswered_requests.empty() ? 0 : &unanswered_requests.front();
	return unanswered_requests;
	}

/**
 * HTTP_ReplyLine() function parses HTTP response line. It is used to extract HTTP status code.
 */
int HTTP_Analyzer::HTTP_ReplyLine(const char* line, const char* end_of_line)
	{
	const char* rest;

	if ( ! (rest = PrefixMatch(line, end_of_line, "HTTP/")) )
		{
		// ##TODO: some server replies with an HTML document
		// without a status line and a MIME header, when the
		// request is malformed.
//		HTTP_Event("bad_HTTP_reply", new_string_val(line, end_of_line));
		return 0;
		}

	SetVersion(reply_version, HTTP_Version(end_of_line - rest, rest));

	for ( ; rest < end_of_line; ++rest )
		if ( is_lws(*rest) )
			break;

	if ( rest >= end_of_line )
		{
//		HTTP_Event("HTTP_reply_code_missing", new_string_val(line, end_of_line));
		return 0;
		}

	rest = skip_whitespace(rest, end_of_line);

	if ( rest + 3 > end_of_line )
		{
//		HTTP_Event("HTTP_reply_code_missing", new_string_val(line, end_of_line));
		return 0;
		}

	reply_code = HTTP_ReplyCode(rest);

	for ( rest += 3; rest < end_of_line; ++rest )
		if ( is_lws(*rest) )
			break;

	if ( rest >= end_of_line )
		{
		reply_reason_phrase.clear();
//		HTTP_Event("HTTP_reply_reason_phrase_missing", new_string_val(line, end_of_line));
		// Tolerate missing reason phrase?
		return 1;
		}

	rest = skip_whitespace(rest, end_of_line);
	reply_reason_phrase.assign(rest, end_of_line - rest);

	return 1;
	}

/**
 * HTTP_ReplyCode() function converts string HTTP status code from string back to integer.
 *
 * @param code_str points to the HTTP vesion string.
 * @return numeric value of HTTP status code.
 */
int HTTP_Analyzer::HTTP_ReplyCode(const char* code_str)
	{
	if ( isdigit(code_str[0]) && isdigit(code_str[1]) && isdigit(code_str[2]) )
		return	(code_str[0] - '0') * 100 +
			(code_str[1] - '0') * 10 +
			(code_str[2] - '0');
	else
		return 0;
	}

int HTTP_Analyzer::ExpectReplyMessageBody()
	{
	// RFC 2616:
	//
	//     For response messages, whether or not a message-body is included with
	//     a message is dependent on both the request method and the response
	//     status code (section 6.1.1). All responses to the HEAD request method
	//     MUST NOT include a message-body, even though the presence of entity-
	//     header fields might lead one to believe they do. All 1xx
	//     (informational), 204 (no content), and 304 (not modified) responses
	//     MUST NOT include a message-body. All other responses do include a
	//     message-body, although it MAY be of zero length.

	const char * method = UnansweredRequestMethod();

	if ( method && strcasecmp(method, "HEAD") == 0 )
		return HTTP_BODY_NOT_EXPECTED;

	if ( (reply_code >= 100 && reply_code < 200) ||
	     reply_code == 204 || reply_code == 304 )
		return HTTP_BODY_NOT_EXPECTED;

	return HTTP_BODY_EXPECTED;
	}

int HTTP_Analyzer::SubmitAllHeaders(bool is_orig)
{
	if (reply_code >= 100 && reply_code < 200)
	{
		return HTTP_BODY_MAYBE;
	}

	if (is_orig)
	{
		//request_headers = hlist;
		//if (current_post)
		//{
		//	delete current_post;
		//	current_post = NULL;
		//}
		// make sure to reset all collected response data ???
	} else {
		//Dump();
	}
	return HTTP_BODY_MAYBE;
}

/**
 * HTTP_Header() function is called from HTTP_Entity on each new HTTP request header.
 */
void HTTP_Analyzer::HTTP_Header(bool is_orig, MIME_Header* h)
	{
#if 0
	// ### Only call ParseVersion if we're tracking versions:
	if ( strcasecmp_n(h->get_name(), "server") == 0 )
		ParseVersion(h->get_value(),
				(is_orig ? Conn()->OrigAddr() : Conn()->RespAddr()), false);

	else if ( strcasecmp_n(h->get_name(), "user-agent") == 0 )
		ParseVersion(h->get_value(),
				(is_orig ? Conn()->OrigAddr() : Conn()->RespAddr()), true);
#endif

	// To be "liberal", we only look at "keep-alive" on the client
	// side, and if seen assume the connection to be persistent.
	// This seems fairly safe - at worst, the client does indeed
	// send additional requests, and the server ignores them.
	if ( is_orig && h->get_name().length == 10 && strcasecmp_n(h->get_name(), "connection") == 0 )
		{
		if ( strcasecmp_n(h->get_value_token(), "keep-alive") == 0 )
			keep_alive = 1;
		}

	if ( ! is_orig && h->get_name().length == 10 )
	{
		if (strcasecmp_n(h->get_name(), "connection") == 0 )
		{
			if ( strcasecmp_n(h->get_value_token(), "close") == 0 )
		        connection_close = 1;
		}
		else if ( strcasecmp_n(h->get_name(), "set-cookie") == 0)
		{
			if (!set_cookie.empty())
				set_cookie.append("; ");
			set_cookie.append(h->get_value().data, h->get_value().length);
		}

	}

	}

/**
 * ParseVersion() function can be used to extract server version. Currently not used.
 */
void HTTP_Analyzer::ParseVersion(data_chunk_t ver, const IPAddr& host,
				bool user_agent)
	{
	int len = ver.length;
	const char* data = ver.data;

//	if ( software_unparsed_version_found )
//		Conn()->UnparsedVersionFoundEvent(host, data, len, this);

	// The RFC defines:
	//
	//	product		= token ["/" product-version]
	//	product-version = token
	//	Server		= "Server" ":" 1*( product | comment )

	int offset;
	data_chunk_t product, product_version;
	int num_version = 0;

	while ( len > 0 )
		{
		// Skip white space.
		while ( len && is_lws(*data) )
			{
			++data;
			--len;
			}

		// See if a comment is coming next. For User-Agent,
		// we parse it, too.
		if ( user_agent && len && *data == '(' )
			{
			// Find end of comment.
			const char* data_start = data;
			const char* eoc =
				data + MIME_skip_lws_comments(len, data);

			// Split into parts.
			// (This may get confused by nested comments,
			// but we ignore this for now.)
			const char* eot;
			++data;
			while ( 1 )
				{
				// Eat spaces.
				while ( data < eoc && is_lws(*data) )
					++data;

				// Find end of token.
				for ( eot = data;
				      eot < eoc && *eot != ';' && *eot != ')';
				      ++eot )
					;

				if ( eot == eoc )
					break;

				// Delete spaces at end of token.
				for ( ; eot > data && is_lws(*(eot-1)); --eot )
					;

//				if ( data != eot && software_version_found )
//					Conn()->VersionFoundEvent(host, data, eot - data, this);
				data = eot + 1;
				}

			len -= eoc - data_start;
			data = eoc;
			continue;
			}

		offset = MIME_get_slash_token_pair(len, data,
						&product, &product_version);
		if ( offset < 0 )
			{
			// I guess version detection is best-effort,
			// so we do not complain in the final version
			if ( num_version == 0 )
				;
//				HTTP_Event("bad_HTTP_version", new_string_val(len, data));

			// Try to simply skip next token.
			offset = MIME_get_token(len, data, &product);
			if ( offset < 0 )
				break;

			len -= offset;
			data += offset;
			}

		else
			{
			len -= offset;
			data += offset;

/*
			int version_len =
				product.length + 1 + product_version.length;

			char* version_str = new char[version_len+1];
			char* s = version_str;

			memcpy(s, product.data, product.length);

			s += product.length;
			*(s++) = '/';

			memcpy(s, product_version.data, product_version.length);

			s += product_version.length;
			*s = 0;

//			if ( software_version_found )
//				Conn()->VersionFoundEvent(host,	version_str,
//							version_len, this);

			delete [] version_str;
*/
			++num_version;
			}
		}
	}

void HTTP_Analyzer::HTTP_EntityData(bool is_orig, const unsigned char *entity_data, size_t entity_size )
	{
	if (entity_size == 0)
		return;
	if (!entity_data)
		return;
	if (is_orig == true)
		{
			DEBUG_MSG("all post data request received[" SIZET_FMT "]\n", entity_size);
			// make sure that previous object is deleted !!!
			//if (current_post != NULL)
			//	delete current_post;
			current_post = entity_data;
			current_post_size = entity_size;
			return;
		}
	DEBUG_MSG("all response received[" SIZET_FMT "]\n", entity_size);
	if (entity_size < 600)
	{
		DEBUG_MSG("data:\n%s\n", (const char*)entity_data);
	}

	//delete entity_data;
	}

// Calls request/reply done
void HTTP_Analyzer::HTTP_MessageDone(bool is_orig, HTTP_Message* /* message */)
	{
	if ( is_orig )
		RequestMade(0, "message ends normally");
	else
		ReplyMade(0, "message ends normally");
	}

void HTTP_Analyzer::InitHTTPMessage(bool is_orig, int expect_body, int64_t init_header_length)
	{
	HTTP_Message* message = is_orig ? &request_message : &reply_message;
	if ( message->Active() )
		{
		if ( ! message->Finished()) 
			Weird("HTTP_overlapping_messages");

		// try to dump request if we got headers
		Dump();
		if (is_orig)
			{
			DEBUG_MSG("reset request message object!\n");
			}
		else
			{
			DEBUG_MSG("reset reply message object!\n");
			}
		//delete message;
		}
	if (is_orig)
		{
		DEBUG_MSG("%.6f init http request message\n", network_time);
		}
	else
		{
		DEBUG_MSG("%.6f init http reply message\n", network_time);
		}
	//message = new HTTP_Message(this, cl, is_orig, expect_body,
	//				init_header_length);
	message->Reset(expect_body, init_header_length, true);
	}

void HTTP_Analyzer::SkipEntityData(bool is_orig)
	{
	HTTP_Message* msg = is_orig ? &request_message : &reply_message;

	if ( msg )
		msg->SkipEntityData();
	}

int is_reserved_URI_char(unsigned char ch)
	{ // see RFC 2396 (definition of URI)
	return strchr(";/?:@&=+$,", ch) != 0;
	}

int is_unreserved_URI_char(unsigned char ch)
	{ // see RFC 2396 (definition of URI)
	return isalnum(ch) || strchr("-_.!~*\'()", ch) != 0;
	}

void escape_URI_char(unsigned char ch, unsigned char*& p)
	{
	*p++ = '%';
	*p++ = encode_hex((ch >> 4) & 0xf);
	*p++ = encode_hex(ch & 0xf);
	}

void unescape_URI(const u_char* line, const u_char* line_end,
						  TCP_Analyzer* analyzer, std::string & res)
	{
	static unsigned char default_buffer[2048];
	unsigned char * decoded_URI = (unsigned char*)&default_buffer;
	char * URI_p = (char*) decoded_URI;
	size_t original_length = line_end - line + 1;

	if (original_length >= sizeof(default_buffer))
	{
		decoded_URI = new u_char[line_end - line + 1];
		URI_p = (char*) decoded_URI;
	}

	// An 'unescaped_special_char' here means a character that *should*
	// be escaped, but isn't in the URI.  A control characters that
	// appears directly in the URI would be an example.  The RFC implies
	// that if we do not unescape the URI that we see in the trace, every
	// character should be a printable one -- either reserved or unreserved
	// (or '%').
	//
	// Counting the number of unescaped characters and generating a weird
	// event on URI's with unescaped characters (which are rare) will
	// let us locate strange-looking URI's in the trace -- those URI's
	// are often interesting.
	int unescaped_special_char = 0;

	while ( line < line_end )
		{
		if ( *line == '%' )
			{
			++line;

			if ( line == line_end )
				{
				// How to deal with % at end of line?
				// *URI_p++ = '%';
				if ( analyzer )
					analyzer->Weird("illegal_%_at_end_of_URI");
				break;
				}

			else if ( *line == '%' )
				{
				// Double '%' might be either due to
				// software bug, or more likely, an
				// evasion (e.g. used by Nimda).
				// *URI_p++ = '%';
				if ( analyzer )
					analyzer->Weird("double_%_in_URI");
				--line;	// ignore the first '%'
				}

			else if ( isxdigit(line[0]) && isxdigit(line[1]) )
				{
				*URI_p++ = (decode_hex(line[0]) << 4) +
					   decode_hex(line[1]);
				++line; // place line at the last hex digit
				}

			else
				{
				if ( analyzer )
					{
						DEBUG_MSG("unescaped_%%_in_URI");
						//analyzer->Weird("unescaped_%_in_URI");
					}
				*URI_p++ = '%';	// put back initial '%'
				*URI_p++ = *line;	// take char w/o interp.
				}
			}

		else
			{
			if ( ! is_reserved_URI_char(*line) &&
			     ! is_unreserved_URI_char(*line) )
				// Count these up as a way to compress
				// the corresponding Weird event to a
				// single instance.
				++unescaped_special_char;
			*URI_p++ = *line;
			}

		++line;
		}

	URI_p[0] = 0;

	if ( unescaped_special_char && analyzer )
	{
		//analyzer->Weird("unescaped_special_URI_char");
		DEBUG_MSG("unescaped_special_URI_char");
	}

	res.assign((char *)decoded_URI, (unsigned char *)URI_p - decoded_URI);
	if (decoded_URI != (unsigned char*)&default_buffer)
		delete[] decoded_URI;
	}
