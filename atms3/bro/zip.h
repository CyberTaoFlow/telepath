// See the file "COPYING" in the main distribution directory for copyright.

#ifndef zip_h
#define zip_h

#include "config.h"

#include "zlib.h"
class HTTP_Entity;
//#include "http.h"

/**
 * ZIP_Analyzer class is used to decompress compressed HTTP responses and send it back down using the http->DeliverBody() function.
 */
class ZIP_Analyzer /* : public TCP_SupportAnalyzer */ {
public:
	enum Method { GZIP, DEFLATE };
	
	/// Object constructor
	ZIP_Analyzer(HTTP_Entity * http, Method method = GZIP);
	/// Object destructor
	~ZIP_Analyzer();
	/// End of decompression
	void Done();
	/// This function decompresses HTTP response and sends clear text using the http->DeliverBody() function.
	void DeliverStream(int len, const u_char* data, bool orig);

protected:
	/// http is a parent HTTP_Entity object
	HTTP_Entity * http;
	enum { NONE, ZIP_OK, ZIP_FAIL };
	z_stream zip;
	int zip_status;
	Method method;
};

#endif
