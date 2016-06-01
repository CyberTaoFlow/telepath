// See the file "COPYING" in the main distribution directory for copyright.

#include "zip.h"
#include "http.h"

ZIP_Analyzer::ZIP_Analyzer(HTTP_Entity * _http, Method arg_method)
/* : TCP_SupportAnalyzer(AnalyzerTag::Zip, conn, orig) */
	{
	http = _http;
	memset(&zip, 0, sizeof(zip));
	zip_status = Z_OK;
	method = arg_method;

	// "15" here means maximum compression.  "32" is a gross overload
	// hack that means "check it for whether it's a gzip file".  Sheesh.
	zip_status = inflateInit2(&zip, 15 + 32);
	if ( zip_status != Z_OK )
		{
		//http->Weird("inflate_init_failed");
		}
	}

ZIP_Analyzer::~ZIP_Analyzer()
	{
	}

void ZIP_Analyzer::Done()
	{
	//Analyzer::Done();

	inflateEnd(&zip);
	}

const static unsigned int unzip_size = 10240*2;
static unsigned char unzipbuf[unzip_size];

void ZIP_Analyzer::DeliverStream(int len, const u_char* data, bool orig_not_used)
	{
//	TCP_SupportAnalyzer::DeliverStream(len, data, orig);

	if ( ! len || zip_status != Z_OK )
		return;

	zip.next_in = (unsigned char*) data;
	zip.avail_in = len;

	do
		{
		zip.next_out = unzipbuf;
		zip.avail_out = unzip_size;

		zip_status = inflate(&zip, Z_SYNC_FLUSH);

		if ( zip_status != Z_STREAM_END &&
		     zip_status != Z_OK &&
		     zip_status != Z_BUF_ERROR )
			{
			//http->Weird("inflate_failed");
			inflateEnd(&zip);
			break;
			}

		int have = unzip_size - zip.avail_out;
		if ( have )
			http->DeliverBody(have, (char*) unzipbuf, false);

//			http->ForwardStream(have, unzipbuf, orig);

		if ( zip_status == Z_STREAM_END )
			{
			inflateEnd(&zip);
			break;
			}

		zip_status = Z_OK;
		}
	while ( zip.avail_out == 0 );

	}
