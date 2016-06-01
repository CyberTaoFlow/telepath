#ifndef mime_h
#define mime_h

#include <assert.h>
//#include <openssl/md5.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <queue>
using namespace std;

#include "base64.h"
//#include "BroString.h"
//#include "Analyzer.h"
#include "reporter.h"

// MIME: Multipurpose Internet Mail Extensions
// Follows RFC 822 & 2822 (Internet Mail), 2045-2049 (MIME)
// See related files: SMTP.h and SMTP.cc

// MIME Constants

enum {
	HTTP_BODY_NOT_EXPECTED,
	HTTP_BODY_EXPECTED,
	HTTP_BODY_MAYBE,
};


#define HT	'\011'
#define SP	'\040'
#define CR	'\015'
#define LF	'\012'

enum MIME_CONTENT_TYPE {
	CONTENT_TYPE_MULTIPART,
	CONTENT_TYPE_MESSAGE,
	CONTENT_TYPE_TEXT,
	CONTENT_TYPE_OTHER2, // used instaed of multipart !
	CONTENT_TYPE_OTHER,	// image | audio | video | application | <other>
};

enum MIME_EVENT_TYPE {
	MIME_EVENT_ILLEGAL_FORMAT,
	MIME_EVENT_ILLEGAL_ENCODING,
	MIME_EVENT_CONTENT_GAP,
	MIME_EVENT_OTHER,
};



// MIME data structures.

class MIME_Header;
class MIME_Body;
class MIME_Entity;	// an "entity" contains headers and a body
class MIME_Message;
class HTTP_Message;

class MIME_Header {
public:
	MIME_Header();
	~MIME_Header();
	void Set(std::string & hl);

	data_chunk_t & get_name() { return name; }
	data_chunk_t & get_value() { return value; }

	data_chunk_t & get_value_token();
	data_chunk_t & get_value_after_token();

protected:
	int get_first_token();

	std::string lines;
	data_chunk_t name;
	data_chunk_t value;
	data_chunk_t value_token, rest_value;
};


// declare(PList, MIME_Header);
typedef vector<MIME_Header*> MIME_HeaderList;

class MIME_Entity {
public:
	MIME_Entity(HTTP_Message* output_message, MIME_Entity* parent_entity);
	virtual ~MIME_Entity();
	void Reset();

	virtual void Deliver(int len, const char* data, int trailing_CRLF);
	virtual void EndOfData();

	MIME_Entity* Parent() const { return parent; }
	int MIMEContentType() const { return content_type; }
	void SetContentType(int c_type) { content_type = c_type; }
	void SetCollectReply(bool value) { collect_reply = value; }
	//std::string* ContentType() const { return &content_type_str; }
	//std::string* ContentSubType() const { return &content_subtype_str; }
	int ContentTransferEncoding() const { return content_encoding; }

protected:

	// {begin, continuation, end} of a header.
	void NewHeader(int len, const char* data);
	void ContHeader(int len, const char* data);
	void FinishHeader();

	void ParseMIMEHeader(MIME_Header* h);
	int LookupMIMEHeaderName(data_chunk_t name);
	int ParseContentTypeField(MIME_Header* h);
	int ParseContentEncodingField(MIME_Header* h);
	int ParseFieldParameters(int len, const char* data);

	void ParseContentType(data_chunk_t type, data_chunk_t sub_type);
	void ParseContentEncoding(data_chunk_t encoding_mechanism);
	void ParseParameter(data_chunk_t attr, data_chunk_t val);

	void BeginBody();
	void NewDataLine(int len, const char* data, int trailing_CRLF);

	int CheckBoundaryDelimiter(int len, const char* data);
	void DecodeDataLine(int len, const char* data, int trailing_CRLF);
	void DecodeBinary(int len, const char* data, int trailing_CRLF);
	void DecodeQuotedPrintable(int len, const char* data);
	void DecodeBase64(int len, const char* data);
	void StartDecodeBase64();
	void FinishDecodeBase64();

	int GetDataBuffer();
	void DataOctet(char ch);
	void DataOctets(int len, const char* data);
	void SubmitData(int len, const char* buf);

	virtual void SubmitHeader(MIME_Header* h);
	// Submit all headers in member "headers".
	virtual int SubmitAllHeaders();

	virtual MIME_Entity* NewChildEntity() { return new MIME_Entity(message, this); }
	void BeginChildEntity();
	void EndChildEntity();

	void IllegalFormat(const char* explanation);
	void IllegalEncoding(const char* explanation);

	void DebugPrintHeaders();

	int in_header;
	int end_of_data;
	bool collect_reply;
	std::string current_header_line;
	int current_field_type;
	int need_to_parse_parameters;
	MIME_Header h;

	std::string content_type_str;
	std::string content_subtype_str;
	std::string content_encoding_str;
	std::string multipart_boundary;

	int content_type, content_subtype, content_encoding;

	//MIME_HeaderList* headers;
	MIME_Entity* parent;
	MIME_Entity* current_child_entity;

	Base64Decoder* base64_decoder;

	int data_buf_length;
	char* data_buf_data;
	int data_buf_offset;

	//MIME_Message* message;
	HTTP_Message* message;
};

// The reason I separate MIME_Message as an abstract class is to
// present the *interface* separated from its implementation to
// generate Bro events.

class MIME_Message {
public:
	MIME_Message(/* Analyzer* arg_analyzer */)
		{
		// Cannot initialize top_level entity because we do
		// not know its type yet (MIME_Entity / MIME_Mail /
		// etc.).
		top_level = 0;
		finished = 0;
		//analyzer = arg_analyzer;
		}

	virtual ~MIME_Message()
		{
		if ( ! finished )
			reporter->InternalError("Done() must be called before destruction");
		}

	virtual void Done()	{ finished = 1; }

	int Finished() const	{ return finished; }

	virtual void Deliver(int len, const char* data, int trailing_CRLF)
		{
		if (top_level)
			top_level->Deliver(len, data, trailing_CRLF);
		}

//	Analyzer* GetAnalyzer() const	{ return analyzer; }

	// Events generated by MIME_Entity
	virtual void BeginEntity(MIME_Entity*) = 0;
	virtual void EndEntity(MIME_Entity*) = 0;
	virtual void SubmitHeader(MIME_Header* h) = 0;
	virtual int SubmitAllHeaders() = 0;
	virtual void SubmitData(int len, const char* buf) = 0;
	virtual int RequestBuffer(int* plen, char** pbuf) = 0;
	virtual void SubmitEvent(int event_type, const char* detail) = 0;

protected:
	//Analyzer* analyzer;

	MIME_Entity* top_level;
	int finished;
};

extern int is_null_data_chunk(data_chunk_t b);
extern int fputs(data_chunk_t b, FILE* fp);
extern int strcasecmp_n(data_chunk_t & s, const char* t);
extern int is_lws(char ch);
extern int MIME_is_field_name_char(char ch);
extern int MIME_count_leading_lws(int len, const char* data);
extern int MIME_count_trailing_lws(int len, const char* data);
extern int MIME_skip_comments(int len, const char* data);
extern int MIME_skip_lws_comments(int len, const char* data);
extern int MIME_get_token(int len, const char* data, data_chunk_t* token);
extern int MIME_get_slash_token_pair(int len, const char* data, data_chunk_t* first, data_chunk_t* second);
extern int MIME_get_value(int len, const char* data, std::string*& buf);
extern int MIME_get_field_name(int len, const char* data, data_chunk_t* name);
extern std::string* MIME_decode_quoted_pairs(data_chunk_t buf);

#endif
