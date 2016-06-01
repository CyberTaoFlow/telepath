#include "config.h"

//#include "NetVar.h"
#include "mime.h"
//#include "Event.h"
#include "reporter.h"
#include "http.h"
//#include "digest.h"
#include <algorithm>

// Here are a few things to do:
//
// 1. Add a Bro internal function 'stop_deliver_data_of_entity' so
// that the engine does not decode and deliver further data for the
// entity (which may speed up the engine by avoiding copying).
//
// 2. Better support for structured header fields, in particular,
// headers of form: <name>=<value>; <param_1>=<param_val_1>;
// <param_2>=<param_val_2>; ... (so that

static const data_chunk_t null_data_chunk = { 0, 0 };

int is_null_data_chunk(data_chunk_t b)
	{
	return b.data == 0;
	}

int fputs(data_chunk_t b, FILE* fp)
	{
	for ( int i = 0; i < b.length; ++i )
		if ( fputc(b.data[i], fp) == EOF )
			return EOF;
	return 0;
	}

data_chunk_t get_data_chunk(std::string* s)
	{
	data_chunk_t b;
	b.length = s->size();
	b.data = (const char*) s->c_str();
	return b;
	}

int mime_header_only = 0;
int mime_decode_data = 1;
int mime_submit_data = 1;

enum MIME_HEADER_FIELDS {
	MIME_CONTENT_TYPE,
	MIME_CONTENT_TRANSFER_ENCODING,
	MIME_FIELD_OTHER,
};

enum MIME_CONTENT_SUBTYPE {
	CONTENT_SUBTYPE_MIXED,		// for multipart
	CONTENT_SUBTYPE_ALTERNATIVE,	// for multipart
	CONTENT_SUBTYPE_DIGEST,		// for multipart

	CONTENT_SUBTYPE_RFC822,		// for message
	CONTENT_SUBTYPE_PARTIAL,	// for message
	CONTENT_SUBTYPE_EXTERNAL_BODY,	// for message

	CONTENT_SUBTYPE_PLAIN,		// for text

	CONTENT_SUBTYPE_OTHER,
};

enum MIME_CONTENT_ENCODING {
	CONTENT_ENCODING_7BIT,
	CONTENT_ENCODING_8BIT,
	CONTENT_ENCODING_BINARY,
	CONTENT_ENCODING_QUOTED_PRINTABLE,
	CONTENT_ENCODING_BASE64,
	CONTENT_ENCODING_OTHER,
};

enum MIME_BOUNDARY_DELIMITER {
	NOT_MULTIPART_BOUNDARY,
	MULTIPART_BOUNDARY,
	MULTIPART_CLOSING_BOUNDARY,
};

static const char* MIMEHeaderName[] = {
	"content-type",
	"content-transfer-encoding",
	0,
};

static const char* MIMEContentTypeName[] = {
	"MULTIPART",
	"MESSAGE",
	"TEXT",
	0,
};

static const char* MIMEContentSubtypeName[] = {
	"MIXED",		// for multipart
	"ALTERNATIVE",		// for multipart
	"DIGEST",		// for multipart

	"RFC822",		// for message
	"PARTIAL",		// for message
	"EXTERNAL-BODY",	// for message

	"PLAIN",		// for text

	0,			// other
};

static const char* MIMEContentEncodingName[] = {
	"7BIT",
	"8BIT",
	"BINARY",
	"QUOTED-PRINTABLE",
	"BASE64",
	0,
};

MIME_Header::MIME_Header()
{
	//lines = &hl;
	name = value = value_token = rest_value = null_data_chunk;
}

/*
MIME_Header::MIME_Header(std::string & hl): lines(hl)
	{
	//lines = &hl;
	name = value = value_token = rest_value = null_data_chunk;

	int len = lines.size();
	const char* data = (const char*) lines.c_str();

	int offset = MIME_get_field_name(len, data, &name);
	if ( offset < 0 )
		return;

	len -= offset; data += offset;
	offset = MIME_skip_lws_comments(len, data);

	if ( offset < len && data[offset] == ':' )
		{
		value.length = len - offset - 1;
		value.data = data + offset + 1;
		while ( value.length && isspace(*value.data) )
			{
			--value.length;
			++value.data;
			}
		}
	else
		// malformed header line
		name = null_data_chunk;
	}
*/

MIME_Header::~MIME_Header()
	{
	//delete lines;
	}

void MIME_Header::Set(std::string & hl)
	{
	lines.assign(hl);
	name = value = value_token = rest_value = null_data_chunk;

	int len = lines.size();
	const char* data = (const char*) lines.c_str();

	int offset = MIME_get_field_name(len, data, &name);
	if ( offset < 0 )
		return;

	len -= offset; data += offset;
	offset = MIME_skip_lws_comments(len, data);

	if ( offset < len && data[offset] == ':' )
		{
		value.length = len - offset - 1;
		value.data = data + offset + 1;
		while ( value.length && isspace(*value.data) )
			{
			--value.length;
			++value.data;
			}
		}
	else
		// malformed header line
		name = null_data_chunk;
	}

int MIME_Header::get_first_token()
	{
	if ( MIME_get_token(value.length, value.data, &value_token) >= 0 )
		{
		rest_value.data = value_token.data + value_token.length;
		rest_value.length = value.data + value.length - rest_value.data;
		return 1;
		}
	else
		{
		value_token = rest_value = null_data_chunk;
		return 0;
		}
	}

data_chunk_t & MIME_Header::get_value_token()
	{
	if ( ! is_null_data_chunk(value_token) )
		return value_token;
	get_first_token();
	return value_token;
	}

data_chunk_t & MIME_Header::get_value_after_token()
	{
	if ( ! is_null_data_chunk(rest_value) )
		return rest_value;
	get_first_token();
	return rest_value;
	}

MIME_Entity::MIME_Entity(HTTP_Message* output_message, MIME_Entity* parent_entity)
	{
	current_header_line.reserve(120);
	base64_decoder = 0;
	Reset();
	parent = parent_entity;
	message = output_message;
	if ( parent )
		content_encoding = parent->ContentTransferEncoding();
	}

void MIME_Entity::Reset()
	{
	in_header = 1;
	end_of_data = 0;

	//current_header_line = 0;
	current_field_type = MIME_FIELD_OTHER;

	need_to_parse_parameters = 0;

	content_type_str = "TEXT";
	content_subtype_str = "PLAIN";

	content_type = CONTENT_TYPE_TEXT;
	content_subtype = CONTENT_SUBTYPE_PLAIN;
	content_encoding = CONTENT_ENCODING_OTHER;

	parent = 0;
	current_child_entity = 0;

	data_buf_length = 0;
	data_buf_data = 0;
	data_buf_offset = -1;

	collect_reply = false;

	content_encoding_str.clear();
	current_header_line.clear();
	multipart_boundary.clear();
	//message = 0;
	//headers = NULL;
	FinishDecodeBase64();
	}

MIME_Entity::~MIME_Entity()
	{
//	if ( ! end_of_data )
//		reporter->InternalError("EndOfData must be called before delete a MIME_Entity");

	//delete current_header_line;
	//delete content_type_str;
	//delete content_subtype_str;
	//Unref(content_type_str);
	//Unref(content_subtype_str);
	//delete content_encoding_str;
	//delete multipart_boundary;
	if (base64_decoder)
		delete base64_decoder;
	}

void MIME_Entity::Deliver(int len, const char* data, int trailing_CRLF)
	{
	if ( in_header )
		{
		if ( len == 0 || *data == '\0' )
			{ // an empty line at the end of header fields
			FinishHeader();
			in_header = 0;

			int ret = SubmitAllHeaders();
			// the following code is done on purpuse !!!
			if (ret == HTTP_BODY_NOT_EXPECTED)
				{
					// this object was freed !
					return;
				}

			// Note: it's possible that we are in the
			// trailer of a chunked transfer (see HTTP.cc).
			// In this case, end_of_data will be set in
			// HTTP_Entity::SubmitAllHeaders(), and we
			// should not begin a new body.

			if ( ! end_of_data )
				BeginBody();
			}

		else if ( is_lws(*data) )
			// linear whitespace - a continuing header line
			ContHeader(len, data);
		else
			NewHeader(len, data);
		}
	else
		{
		if ( ! mime_header_only && data )
			NewDataLine(len, data, trailing_CRLF);
		}
	}

void MIME_Entity::BeginBody()
	{
	if ( content_encoding == CONTENT_ENCODING_BASE64 )
		StartDecodeBase64();

	if ( content_type == CONTENT_TYPE_MESSAGE )
		BeginChildEntity();
	}

void MIME_Entity::EndOfData()
	{
	if ( end_of_data )
		return;

	end_of_data = 1;

	if ( in_header )
		{
		FinishHeader();
		in_header = 0;
		SubmitAllHeaders();
		message->SubmitEvent(MIME_EVENT_ILLEGAL_FORMAT,
					"entity body missing");
		}

	else
		{
		if ( current_child_entity != 0 )
			{
			if ( content_type == CONTENT_TYPE_MULTIPART )
				IllegalFormat("multipart closing boundary delimiter missing");
			EndChildEntity();
			}

		if ( content_encoding == CONTENT_ENCODING_BASE64 )
			FinishDecodeBase64();

		if ( data_buf_offset > 0 )
			{
			SubmitData(data_buf_offset, data_buf_data);
			data_buf_offset = -1;
			}
		}

	message->EndEntity (this);
	}

void MIME_Entity::NewDataLine(int len, const char* data, int trailing_CRLF)
	{
	if ( content_type == CONTENT_TYPE_MULTIPART )
		{
		switch ( CheckBoundaryDelimiter(len, data) ) {
			case MULTIPART_BOUNDARY:
				if ( current_child_entity != 0 )
					EndChildEntity();
				BeginChildEntity();
				return;

			case MULTIPART_CLOSING_BOUNDARY:
				if ( current_child_entity != 0 )
					EndChildEntity();
				EndOfData();
				return;
		}
		}

	if ( content_type == CONTENT_TYPE_MULTIPART ||
	     content_type == CONTENT_TYPE_MESSAGE )
		{
		// Here we ignore the difference among 7bit, 8bit and
		// binary encoding, and thus do not need to decode
		// before passing the data to child.

		if ( current_child_entity != 0 )
			// Data before the first or after the last
			// boundary delimiter are ignored
			current_child_entity->Deliver(len, data, trailing_CRLF);
		}
	else
		{
		if ( mime_decode_data )
			DecodeDataLine(len, data, trailing_CRLF);
		}
	}

void MIME_Entity::NewHeader(int len, const char* data)
	{
	FinishHeader();

	if ( len == 0 )
		return;

	ASSERT(! is_lws(*data));

	current_header_line.assign(data, len);
	}

void MIME_Entity::ContHeader(int len, const char* data)
	{
	if ( current_header_line.empty() )
		{
		IllegalFormat("first header line starts with linear whitespace");

		// shall we try it as a new header or simply ignore this line?
		int ws = MIME_count_leading_lws(len, data);
		NewHeader(len - ws, data + ws);
		return;
		}

	current_header_line.append(data, len);
	}

void MIME_Entity::FinishHeader()
	{
	if ( current_header_line.empty() )
		return;

	h.Set(current_header_line);
	current_header_line.clear();

	if ( ! is_null_data_chunk(h.get_name()) )
		{
		ParseMIMEHeader(&h);
		SubmitHeader(&h);
		//if (headers == NULL)
		//	{
		//		headers = new MIME_HeaderList();
		//	}
		//message->headers.push_back(h);
		}
	}

int MIME_Entity::LookupMIMEHeaderName(data_chunk_t name)
	{
	// A linear lookup should be fine for now.
	// header names are case-insensitive (RFC 822, 2822, 2045).

	for ( int i = 0; MIMEHeaderName[i] != 0; ++i )
		if ( strcasecmp_n(name, MIMEHeaderName[i]) == 0 )
			return i;
	return -1;
	}

void MIME_Entity::ParseMIMEHeader(MIME_Header* h)
	{
	if ( h == 0 )
		return;

	current_field_type = LookupMIMEHeaderName(h->get_name());

	switch ( current_field_type ) {
		case MIME_CONTENT_TYPE:
			ParseContentTypeField(h);
			break;

		case MIME_CONTENT_TRANSFER_ENCODING:
			ParseContentEncodingField(h);
			break;
	}
	}

int MIME_Entity::ParseContentTypeField(MIME_Header* h)
	{
	data_chunk_t val = h->get_value();
	int len = val.length;
	const char* data = val.data;

	data_chunk_t ty, subty;
	int offset;

	offset = MIME_get_slash_token_pair(len, data, &ty, &subty);
	if ( offset < 0 )
		{
		IllegalFormat("media type/subtype not found in content type");
		return 0;
		}
	data += offset;
	len -= offset;

	content_type_str.assign(ty.data, ty.length);
	std::transform(content_type_str.begin(),content_type_str.end(),content_type_str.begin(),::tolower);
	content_subtype_str.assign(subty.data, subty.length);
	std::transform(content_subtype_str.begin(),content_subtype_str.end(),content_subtype_str.begin(),::tolower);
	if (content_subtype_str == "html" || content_subtype_str == "xhtml" || content_subtype_str == "xhtml+xml")
	{
		collect_reply = true;
	}
	
	ParseContentType(ty, subty);

	if ( content_type == CONTENT_TYPE_MULTIPART )
	{
		// we do not ned to handle multipart messages here
		content_type = CONTENT_TYPE_OTHER2;
		content_subtype = CONTENT_SUBTYPE_OTHER;
		multipart_boundary.clear();
	}
	return 1;

	// Proceed to parameters.
	if ( need_to_parse_parameters )
		ParseFieldParameters(len, data);

	if ( content_type == CONTENT_TYPE_MULTIPART && multipart_boundary.empty() )
		{
		IllegalFormat("boundary delimiter is not specified for a multipart entity -- content is treated as type application/octet-stream");
		content_type = CONTENT_TYPE_OTHER2;
		content_subtype = CONTENT_SUBTYPE_OTHER;
		}

	return 1;
	}

int MIME_Entity::ParseContentEncodingField(MIME_Header* h)
	{
	data_chunk_t enc;

	enc = h->get_value_token();
	if ( is_null_data_chunk(enc) )
		{
		IllegalFormat("encoding type not found in content encoding");
		return 0;
		}

	content_encoding_str.assign((const char*)enc.data, enc.length);
	ParseContentEncoding(enc);

	if ( need_to_parse_parameters )
		{
		data_chunk_t val = h->get_value_after_token();
		if ( ! is_null_data_chunk(val) )
			ParseFieldParameters(val.length, val.data);
		}

	return 1;
	}

int MIME_Entity::ParseFieldParameters(int len, const char* data)
	{
	data_chunk_t attr;
	std::string* val;

	while ( 1 )
		{
		int offset = MIME_skip_lws_comments(len, data);
		if ( offset < 0 || offset >= len || data[offset] != ';' )
			break;

		++offset;
		data += offset;
		len -= offset;

		offset = MIME_get_token(len, data, &attr);
		if ( offset < 0 )
			{
			IllegalFormat("attribute name not found in parameter specification");
			return 0;
			}

		data += offset;
		len -= offset;

		offset = MIME_skip_lws_comments(len, data);
		if ( offset < 0 || offset >= len || data[offset] != '=' )
			{
			IllegalFormat("= not found in parameter specification");
			continue;
			}

		++offset;
		data += offset;
		len -= offset;

		// token or quoted-string
		offset = MIME_get_value(len, data, val);
		if ( offset < 0 )
			{
			IllegalFormat("value not found in parameter specification");
			continue;
			}

		data += offset;
		len -= offset;

		ParseParameter(attr, get_data_chunk(val));
		delete val;
		}

	return 1;
	}

void MIME_Entity::ParseContentType(data_chunk_t type, data_chunk_t sub_type)
	{
	int i;
	for ( i = 0; MIMEContentTypeName[i]; ++i )
		if ( strcasecmp_n(type, MIMEContentTypeName[i]) == 0 )
			break;

	content_type = i;

	for ( i = 0; MIMEContentSubtypeName[i]; ++i )
		if ( strcasecmp_n(sub_type, MIMEContentSubtypeName[i]) == 0 )
			break;

	content_subtype = i;

	switch ( content_type ) {
		case CONTENT_TYPE_MULTIPART:
		case CONTENT_TYPE_MESSAGE:
			need_to_parse_parameters = 1;
			break;

		default:
			need_to_parse_parameters = 0;
			break;
	}
	}

void MIME_Entity::ParseContentEncoding(data_chunk_t encoding_mechanism)
	{
	int i;
	for ( i = 0; MIMEContentEncodingName[i]; ++i )
		if ( strcasecmp_n(encoding_mechanism,
					MIMEContentEncodingName[i]) == 0 )
			break;

	content_encoding = i;
	}

void MIME_Entity::ParseParameter(data_chunk_t attr, data_chunk_t val)
	{
	switch ( current_field_type ) {
		case MIME_CONTENT_TYPE:
			if ( content_type == CONTENT_TYPE_MULTIPART &&
			     strcasecmp_n(attr, "boundary") == 0 )
				 multipart_boundary.assign((const char*)val.data, val.length);
			break;

		case MIME_CONTENT_TRANSFER_ENCODING:
			break;

		default:
			break;
	}
	}


int MIME_Entity::CheckBoundaryDelimiter(int len, const char* data)
	{
	if ( multipart_boundary.empty() )
		{
		reporter->Warning("boundary delimiter was not specified for a multipart message\n");
		DEBUG_MSG("headers of the MIME entity for debug:\n");
		DebugPrintHeaders();
		return NOT_MULTIPART_BOUNDARY;
		}

	if ( len >= 2 && data[0] == '-' && data[1] == '-' )
		{
		len -= 2; data += 2;

		data_chunk_t delim = get_data_chunk(&multipart_boundary);

		int i;
		for ( i = 0; i < len && i < delim.length; ++i )
			if ( data[i] != delim.data[i] )
				return NOT_MULTIPART_BOUNDARY;

		if ( i < delim.length )
			return NOT_MULTIPART_BOUNDARY;

		len -= i;
		data += i;

		if ( len >= 2 && data[0] == '-' && data[1] == '-' )
			return MULTIPART_CLOSING_BOUNDARY;
		else
			return MULTIPART_BOUNDARY;
		}

	return NOT_MULTIPART_BOUNDARY;
	}


// trailing_CRLF indicates whether an implicit CRLF sequence follows data
// (the CRLF sequence is not included in data).

void MIME_Entity::DecodeDataLine(int len, const char* data, int trailing_CRLF)
	{
	if ( ! mime_submit_data )
		return;

	switch ( content_encoding ) {
		case CONTENT_ENCODING_QUOTED_PRINTABLE:
			DecodeQuotedPrintable(len, data);
			break;

		case CONTENT_ENCODING_BASE64:
			DecodeBase64(len, data);
			break;

		case CONTENT_ENCODING_7BIT:
		case CONTENT_ENCODING_8BIT:
		case CONTENT_ENCODING_BINARY:
		case CONTENT_ENCODING_OTHER:
			DecodeBinary(len, data, trailing_CRLF);
			break;
	}
	}

void MIME_Entity::DecodeBinary(int len, const char* data, int trailing_CRLF)
	{
	DataOctets(len, data);

	if ( trailing_CRLF )
		{
		DataOctet(CR);
		DataOctet(LF);
		}
	}

void MIME_Entity::DecodeQuotedPrintable(int len, const char* data)
	{
	// Ignore trailing HT and SP.
	int i;
	for ( i = len - 1; i >= 0; --i )
		if ( data[i] != HT && data[i] != SP )
			break;

	int end_of_line = i;
	int soft_line_break = 0;

	for ( i = 0; i <= end_of_line; ++i )
		{
		if ( data[i] == '=' )
			{
			if ( i == end_of_line )
				soft_line_break = 1;
			else
				{
				int legal = 0;
				if ( i + 2 < len )
					{
					int a, b;
					a = decode_hex(data[i+1]);
					b = decode_hex(data[i+2]);

					if ( a >= 0 && b >= 0 )
						{
						DataOctet((a << 4) + b);
						legal = 1;
						}
					}

				if ( ! legal )
					{
					// Follows suggestions for a robust
					// decoder. See RFC 2045 page 22.
					IllegalEncoding("= is not followed by two hexadecimal digits in quoted-printable encoding");
					DataOctet(data[i]);
					}
				}
			}

		else if ( (data[i] >= 33 && data[i] <= 60) ||
			   // except controls, whitespace and '='
			  (data[i] >= 62 && data[i] <= 126) )
			DataOctet(data[i]);

		else if ( data[i] == HT || data[i] == SP )
			DataOctet(data[i]);

		else
			{
			IllegalEncoding(fmt("control characters in quoted-printable encoding: %d", (int) (data[i])));
			DataOctet(data[i]);
			}
		}

	if ( ! soft_line_break )
		{
		DataOctet(CR);
		DataOctet(LF);
		}
	}

void MIME_Entity::DecodeBase64(int len, const char* data)
	{
	int rlen;
	char rbuf[128];

	while ( len > 0 )
		{
		rlen = 128;
		char* prbuf = rbuf;
		int decoded = base64_decoder->Decode(len, data, &rlen, &prbuf);
		if ( prbuf != rbuf )
			reporter->InternalError("buffer pointer modified in base64 decoding");
		DataOctets(rlen, rbuf);
		len -= decoded; data += decoded;
		}
	}

void MIME_Entity::StartDecodeBase64()
	{
	if ( base64_decoder )
		reporter->InternalError("previous Base64 decoder not released!");

	base64_decoder = new Base64Decoder( /* message->GetAnalyzer() */);
	}

void MIME_Entity::FinishDecodeBase64()
	{
	if ( ! base64_decoder )
		return;

	int rlen = 128;
	char rbuf[128];
	char* prbuf = rbuf;

	if ( base64_decoder->Done(&rlen, &prbuf) )
		{ // some remaining data
		if ( prbuf != rbuf )
			reporter->InternalError("buffer pointer modified in base64 decoding");
		if ( rlen > 0 )
			DataOctets(rlen, rbuf);
		}

	delete base64_decoder;
	base64_decoder = 0;
	}

int MIME_Entity::GetDataBuffer()
	{
	int ret = message->RequestBuffer(&data_buf_length, &data_buf_data);
	if ( ! ret || data_buf_length == 0 || data_buf_data == 0 )
		{
		// reporter->InternalError("cannot get data buffer from MIME_Message", "");
		return 0;
		}

	data_buf_offset = 0;
	return 1;
	}

void MIME_Entity::DataOctet(char ch)
	{
	if ( data_buf_offset < 0 && ! GetDataBuffer() )
		return;

	if ( data_buf_offset >= data_buf_length && data_buf_length != 0 )
	{
		return;
	}

	data_buf_data[data_buf_offset++] = ch;
	//++data_buf_offset;

	data_buf_data[data_buf_offset] = '\0';

	//if ( data_buf_offset == data_buf_length )
	//	{
	//	SubmitData(data_buf_length, data_buf_data);
	//	data_buf_offset = -1;
	//	}
	}

void MIME_Entity::SubmitData(int len, const char* buf)
	{
	message->SubmitData(len, buf);
	}

void MIME_Entity::DataOctets(int len, const char* data)
	{
	// sanity check
	// we are using static buffers to receive data
	// make sure not to collect data bigger than our buffer

	if ( data_buf_offset >= data_buf_length && data_buf_length != 0 )
	{
		return;
	}

	//while ( len > 0 )
	if (len > 0)
		{
		if ( data_buf_offset < 0 && ! GetDataBuffer() )
			return;

		int n = min(data_buf_length - data_buf_offset, len);
		memcpy(data_buf_data + data_buf_offset, data, n);
		data += n;
		data_buf_offset += n;
		len -= n;
		// place zero to the end of the buffer!
		// we have one byte reserved for this
		data_buf_data[data_buf_offset] = 0;

		if ( data_buf_offset == data_buf_length )
			{
			// we do not submit collected data here !
			//SubmitData(data_buf_length, data_buf_data);
			//data_buf_offset = -1;
			return;
			}
		}
	}

void MIME_Entity::SubmitHeader(MIME_Header* h)
	{
	message->SubmitHeader(h);
	}

int MIME_Entity::SubmitAllHeaders()
	{
	return message->SubmitAllHeaders();
	}

void MIME_Entity::BeginChildEntity()
	{
	ASSERT(current_child_entity == 0);
	current_child_entity = NewChildEntity();
	message->BeginEntity(current_child_entity);
	}

void MIME_Entity::EndChildEntity()
	{
	ASSERT(current_child_entity != 0);

	current_child_entity->EndOfData();
	//if (current_child_entity == top_level)
	//	top_level = NULL;
	delete current_child_entity;
	current_child_entity = 0;
	}

void MIME_Entity::IllegalFormat(const char* explanation)
	{
	message->SubmitEvent(MIME_EVENT_ILLEGAL_FORMAT, explanation);
	}

void MIME_Entity::IllegalEncoding(const char* explanation)
	{
	message->SubmitEvent(MIME_EVENT_ILLEGAL_ENCODING, explanation);
	}

void MIME_Entity::DebugPrintHeaders()
	{
#ifdef DEBUG_BRO
	for ( unsigned int i = 0; i < headers.size(); ++i )
		{
		MIME_Header* h = headers[i];

		DEBUG_fputs(h->get_name(), stderr);
		DEBUG_MSG(":\"");
		DEBUG_fputs(h->get_value(), stderr);
		DEBUG_MSG("\"\n");
		}
#endif
	}

int strcasecmp_n(data_chunk_t & s, const char* t)
	{
	return strcasecmp_n(s.length, s.data, t);
	}

int is_lws(char ch)
	{
	return ch == 9 || ch == 32;
	}

int MIME_count_leading_lws(int len, const char* data)
	{
	int i;
	for ( i = 0; i < len; ++i )
		if ( ! is_lws(data[i]) )
			break;
	return i;
	}

int MIME_count_trailing_lws(int len, const char* data)
	{
	int i;
	for ( i = 0; i < len; ++i )
		if ( ! is_lws(data[len - 1 - i]) )
			break;
	return i;
	}

// See RFC 2822, page 11
int MIME_skip_comments(int len, const char* data)
	{
	if ( len == 0 || data[0] != '(' )
		return 0;

	int par = 0;
	for ( int i = 0; i < len; ++i )
		{
		switch ( data[i] ) {
		case '(':
			++par;
			break;

		case ')':
			--par;
			if ( par == 0 )
				return i + 1;
			break;

		case '\\':
			++i;
			break;
		}
		}

	return len;
	}

// Skip over lws and comments, but not tspecials. Do not use this
// function in quoted-string or comments.
int MIME_skip_lws_comments(int len, const char* data)
	{
	int i = 0;
	while ( i < len )
		{
		if ( is_lws(data[i]) )
			++i;
		else
			{
			if ( data[i] == '(' )
				i += MIME_skip_comments(len - i, data + i);
			else
				return i;
			}
		}

	return len;
	}

int MIME_get_field_name(int len, const char* data, data_chunk_t* name)
	{
	int i = MIME_skip_lws_comments(len, data);
	while ( i < len )
		{
		int j;
		if ( MIME_is_field_name_char(data[i]) )
			{
			name->data = data + i;

			for ( j = i; j < len; ++j )
				if ( ! MIME_is_field_name_char(data[j]) )
					break;

			name->length = j - i;
			return j;
			}

		j = MIME_skip_lws_comments(len - i, data + i);
		i += (j > 0) ? j : 1;
		}

	return -1;
	}

// See RFC 2045, page 12.
int MIME_is_tspecial (char ch)
	{
	return ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '@' ||
	       ch == ',' || ch == ';' || ch == ':' || ch == '\\' || ch == '"' ||
	       ch == '/' || ch == '[' || ch == ']' || ch == '?' || ch == '=';
	}

int MIME_is_field_name_char (char ch)
	{
	return ch >= 33 && ch <= 126 && ch != ':';
	}

int MIME_is_token_char (char ch)
	{
	return ch >= 33 && ch <= 126 && ! MIME_is_tspecial(ch);
	}

// See RFC 2045, page 12.
// A token is composed of characters that are not SPACE, CTLs or tspecials
int MIME_get_token(int len, const char* data, data_chunk_t* token)
	{
	int i = MIME_skip_lws_comments(len, data);
	while ( i < len )
		{
		int j;

		if ( MIME_is_token_char(data[i]) )
			{
			token->data = (data + i);
			for ( j = i; j < len; ++j )
				{
				if ( ! MIME_is_token_char(data[j]) )
					break;
				}

			token->length = j - i;
			return j;
			}

		j = MIME_skip_lws_comments(len - i, data + i);
		i += (j > 0) ? j : 1;
		}

	return -1;
	}

int MIME_get_slash_token_pair(int len, const char* data, data_chunk_t* first, data_chunk_t* second)
	{
	int offset;
	const char* data_start = data;

	offset = MIME_get_token(len, data, first);
	if ( offset < 0 )
		{
		// DEBUG_MSG("first token missing in slash token pair");
		return -1;
		}

	data += offset;
	len -= offset;

	offset = MIME_skip_lws_comments(len, data);
	if ( offset < 0 || offset >= len || data[offset] != '/' )
		{
		// DEBUG_MSG("/ not found in slash token pair");
		return -1;
		}

	++offset;
	data += offset;
	len -= offset;

	offset = MIME_get_token(len, data, second);
	if ( offset < 0 )
		{
		DEBUG_MSG("second token missing in slash token pair");
		return -1;
		}

	data += offset;
	len -= offset;

	return data - data_start;
	}

// See RFC 2822, page 13.
int MIME_get_quoted_string(int len, const char* data, data_chunk_t* str)
	{
	int offset = MIME_skip_lws_comments(len, data);

	len -= offset;
	data += offset;

	if ( len <= 0 || *data != '"' )
		return -1;

	for ( int i = 1; i < len; ++i )
		{
		switch ( data[i] ) {
		case '"':
			str->data = data + 1;
			str->length = i - 1;
			return offset + i + 1;

		case '\\':
			++i;
			break;
		}
		}

	return -1;
	}

int MIME_get_value(int len, const char* data, std::string*& buf)
	{
	int offset = MIME_skip_lws_comments(len, data);

	len -= offset;
	data += offset;

	if ( len > 0 && *data == '"' )
		{
		data_chunk_t str;
		int end = MIME_get_quoted_string(len, data, &str);
		if ( end < 0 )
			return -1;

		buf = MIME_decode_quoted_pairs(str);
		return offset + end;
		}

	else
		{
		data_chunk_t str;
		int end = MIME_get_token(len, data, &str);
		if ( end < 0 )
			return -1;

		//buf = new BroString((const u_char*)str.data, str.length, 1);
		buf = new std::string(str.data, str.length);
		return offset + end;
		}
	}

// Decode each quoted-pair: a '\' followed by a character by the
// quoted character. The decoded string is returned.

std::string * MIME_decode_quoted_pairs(data_chunk_t buf)
	{
	std::string * str = new std::string();
	const char* data = buf.data;
	//char* dest = new char[buf.length+1];
	int j = 0;
	str->resize(buf.length+1, 0);
	char* dest = (char*)str->c_str();

	for ( int i = 0; i < buf.length; ++i )
		if ( data[i] == '\\' )
			{
			if ( ++i < buf.length )
				dest[j++] = data[i];
			else
				{
				// a trailing '\' -- don't know what
				// to do with it -- ignore it.
				}
			}
		else
			dest[j++] = data[i];
	//dest[j] = 0;
	return str;
	//return new std::string(1, (byte_vec) dest, j);
	}

