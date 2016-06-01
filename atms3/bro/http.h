// See the file "COPYING" in the main distribution directory for copyright.

#ifndef http_h
#define http_h

#include <map>
//#include "tcp.h"
//#include "contentline.h"
#include "mime.h"
//#include "binpac_bro.h"
//#include "zip.h"
//#include "ipaddr.h"

const bool http_header = true;
const bool http_all_headers = true;
const bool http_begin_entity = true;
const bool http_end_entity = true;
const bool http_entity_data = true;
const bool http_message_done = true;
const bool http_event = true;
const bool http_stats = true;
const bool FLAGS_use_binpac = false;
const int max_post_size = 10240-3; //1024*100;

// The EXPECT_*_NOTHING states are used to prevent further parsing. Used if a
// message was interrupted.
enum {
	EXPECT_REQUEST_LINE,
	EXPECT_REQUEST_MESSAGE,
	EXPECT_REQUEST_TRAILER,
	EXPECT_REQUEST_NOTHING,
	EXPECT_REQUEST_RESET
};

enum {
	EXPECT_REPLY_LINE,
	EXPECT_REPLY_MESSAGE,
	EXPECT_REPLY_TRAILER,
	EXPECT_REPLY_NOTHING,
};

enum CHUNKED_TRANSFER_STATE {
	NON_CHUNKED_TRANSFER,
	BEFORE_CHUNK,
	EXPECT_CHUNK_SIZE,
	EXPECT_CHUNK_DATA,
	EXPECT_CHUNK_DATA_CRLF,
	EXPECT_CHUNK_TRAILER,
	EXPECT_NOTHING,
};

class ContentLine_Analyzer;
class TCP_Analyzer;
class TCP_Endpoint;
class HTTP_Entity;
class HTTP_Message;
class HTTP_Analyzer;
class ZIP_Analyzer;

class HTTP_Entity : public MIME_Entity {
public:
	HTTP_Entity(HTTP_Message* msg, MIME_Entity* parent_entity,
			int expect_body);
	~HTTP_Entity();
	void Reset(int expect_body);

	void EndOfData();
	void Deliver(int len, const char* data, int trailing_CRLF);
	int Undelivered(int len);
	int64_t BodyLength() const 		{ return body_length; }
	int64_t HeaderLength() const 	{ return header_length; }
	void SkipBody() 		{ deliver_body = 0; }

protected:
//	class UncompressedOutput;
//	friend class UncompressedOutput;
	friend class ZIP_Analyzer;

	HTTP_Message* http_message;
	int chunked_transfer_state;
	int64_t chunked_content_length;
	int64_t content_length;
	int expect_data_length;
	int expect_body;
	int64_t body_length;
	int64_t header_length;
	int deliver_body;
	enum { IDENTITY, GZIP, COMPRESS, DEFLATE } encoding;
	ZIP_Analyzer* zip;

	MIME_Entity* NewChildEntity() { return new HTTP_Entity(http_message, this, 1); }

	void DeliverBody(int len, const char* data, int trailing_CRLF);

	void SubmitData(int len, const char* buf);

	void SetPlainDelivery(int64_t length);

	void SubmitHeader(MIME_Header* h);
	int SubmitAllHeaders();
};

// Finishing HTTP Messages:
//
// HTTP_Entity::SubmitAllHeaders	-> EndOfData (no body)
// HTTP_Entity::Deliver	-> EndOfData (end of body)
// HTTP_Analyzer::Done	-> {Request,Reply}Made (connection terminated)
// {Request,Reply}Made	-> HTTP_Message::Done
// HTTP_Message::Done	-> MIME_Message::Done, EndOfData, HTTP_MessageDone
// MIME_Entity::EndOfData	-> Message::EndEntity
// HTTP_Message::EndEntity	-> Message::Done
// HTTP_MessageDone	-> {Request,Reply}Made

class HTTP_Message /* : public MIME_Message */ {
public:
	HTTP_Message(HTTP_Analyzer* analyzer, ContentLine_Analyzer* cl,
			 bool is_orig, int expect_body = 0, int64_t init_header_length = 0);
	~HTTP_Message();
	void Reset(int expect_body, int64_t init_header_length, bool active_arg = false);

	void Done(const int interrupted, const char* msg);
	void Done() { Done(0, "message ends normally"); }
	void ClearHeaders();
	void Clear();

	int Finished() const	{ return finished; }
	bool Active() const { return active; }
	void Deliver(int len, const char* data, int trailing_CRLF);
	int Undelivered(int len);

	void BeginEntity(MIME_Entity* /* entity */);
	void EndEntity(MIME_Entity* entity);
	void SubmitHeader(MIME_Header* h);
	int SubmitAllHeaders( );
	void SubmitData(int len, const char* buf);
	int RequestBuffer(int* plen, char** pbuf);
	void SubmitAllData();
	void SubmitEvent(int event_type, const char* detail);
	void SubmitTitle(const char * data, size_t len);
	void CheckLoginMsg(const char * reply, size_t len);

	void SubmitTrailingHeaders( );
	void SetPlainDelivery(int64_t length);
	void SetContentType(int c_type);
	void SetCollectReply(bool value);
	void SkipEntityData();

	HTTP_Analyzer* MyHTTP_Analyzer() const
		{ return (HTTP_Analyzer*) analyzer; }

	void Weird(const char* msg);
	bool IsOrig()	{ return is_orig; }

	HTTP_Entity* current_entity;
	//const std::string * current_post;

	std::map<std::string, std::string> headers;

protected:
	HTTP_Analyzer* analyzer;
	ContentLine_Analyzer* content_line;
	bool is_orig;
	bool active;
	int finished;
	HTTP_Entity top_level;
	std::string temp_key;
	std::string temp_value;

	//vector<const std::string*> buffers;

	// Controls the total buffer size within http_entity_data_delivery_size.
	int total_buffer_size;

	int buffer_offset, buffer_size;
	//std::string* data_buffer;
	unsigned char data_buffer[max_post_size+3];

	double start_time;

	int64_t body_length;	// total length of entity bodies
	int64_t header_length;	// total length of headers, including the request/reply line

	// Total length of content gaps that are "successfully" skipped.
	// Note: this might NOT include all content gaps!
	int64_t content_gap_length;

	int InitBuffer(int64_t length);
	void DeliverEntityData();

//	Val* BuildMessageStat(const int interrupted, const char* msg);
};

class HTTP_Analyzer /* : public TCP_ApplicationAnalyzer */ {
public:
	HTTP_Analyzer(TCP_Analyzer * tcp /* Connection* conn */, ContentLine_Analyzer* content_line_orig, ContentLine_Analyzer* content_line_resp);
	~HTTP_Analyzer();
	void Reset();

	void Undelivered(TCP_Endpoint* sender, int seq, int len);

	int SubmitAllHeaders(bool is_orig );
	void HTTP_Header(bool is_orig, MIME_Header* h);
	void HTTP_EntityData(bool is_orig, const unsigned char * entity_data, size_t entity_size);
	void HTTP_MessageDone(bool is_orig, HTTP_Message* message);

	void SkipEntityData(bool is_orig);

	int IsConnectionClose()		{ return connection_close; }
	int HTTP_ReplyCode() const { return reply_code; };

	// Overriden from Analyzer.
	virtual void Done();
	virtual void DeliverStream(int len, const u_char* data, bool orig, ContentLine_Analyzer* content_line);
	virtual void Undelivered(int seq, int len, bool orig, ContentLine_Analyzer* content_line);

	// Overriden from TCP_ApplicationAnalyzer
	virtual void EndpointEOF(bool is_orig);
	virtual void ConnectionFinished(int half_finished);
	virtual void ConnectionReset();
	virtual void PacketWithRST();

	void Weird(const char * msg);
	void ProtocolViolation(const char * msg);
	void ProtocolConfirmation()
	{

	}

	void Dump();
	void SubmitTitle(const char * data, size_t len);
	void CheckLoginMsg(const char * reply, size_t len);

protected:
	friend class ContentLine_Analyzer;
	void GenStats();

	int HTTP_RequestLine(const char* line, const char* end_of_line);
	int HTTP_ReplyLine(const char* line, const char* end_of_line);

	void InitHTTPMessage(bool is_orig, int expect_body, int64_t init_header_length);

	const char* PrefixMatch(const char* line, const char* end_of_line,
				const char* prefix);
	const char* PrefixWordMatch(const char* line, const char* end_of_line,
				const char* prefix);

	int ParseRequest(const char* line, const char* end_of_line);
	double HTTP_Version(int len, const char* data);

	void SetVersion(double& version, double new_version);

	int RequestExpected() const { return num_requests == 0 || keep_alive; }

	void HTTP_Request();
	void HTTP_Reply();

	void RequestMade(const int interrupted, const char* msg);
	void ReplyMade(const int interrupted, const char* msg);
//	void RequestClash(Val* clash_val);

	TCP_Analyzer * tcp;

	const char * UnansweredRequestMethod();

	void ParseVersion(data_chunk_t ver, const IPAddr& host, bool user_agent);
	int HTTP_ReplyCode(const char* code_str);
	int ExpectReplyMessageBody();

	std::string * TruncateURI(std::string* uri);

	int request_state, reply_state;
	int num_requests, num_replies;
	int num_request_lines, num_reply_lines;
	double request_version, reply_version;
	int keep_alive;
	int connection_close;
	int request_ongoing, reply_ongoing;

	const char * request_method;
	const unsigned char * current_post;
	size_t current_post_size;

	// request_URI is in the original form (may contain '%<hex><hex>'
	// sequences).
	std::string request_URI;
	std::string set_cookie;
	// unescaped_URI does not contain escaped sequences.
	//std::string unescaped_URI;

	std::string title;
	std::string login_msg;
	const char * unanswered_requests;

	int reply_code;
	std::string reply_reason_phrase;

	//ContentLine_Analyzer* content_line_orig;
	//ContentLine_Analyzer* content_line_resp;

	HTTP_Message request_message;
	HTTP_Message reply_message;
	//MIME_HeaderList * request_headers;
	//MIME_HeaderList * reply_headers;
};

extern int is_reserved_URI_char(unsigned char ch);
extern int is_unreserved_URI_char(unsigned char ch);
extern void escape_URI_char(unsigned char ch, unsigned char*& p);
extern void unescape_URI(const u_char* line, const u_char* line_end,
								 TCP_Analyzer* analyzer, std::string & res);

#endif
