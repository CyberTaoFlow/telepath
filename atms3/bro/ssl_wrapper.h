#ifndef _SSL_WRAPPER_
#define _SSL_WRAPPER_

#include <openssl/rsa.h>
#include "../ssl/ssl_decoder.h"
#include "analyzer.h"
#include <string>

class ContentLine_Analyzer;
//class SSLDecoder;

//extern SSLDecoder * get_ssl_decoder(const char * server_ip, unsigned int port);

/**
 * SSLWrapper_Analyzer class is used as a wrapper between SSL decoding layer and connection object. It is used as following:
 * TCP Reasembly layer -> SSLWrapper_Analyzer -> SSL Decoder -> SSLWrapper_Analyzer -> HTTP layer (ContentLine)
 */
class SSLWrapper_Analyzer : public Analyzer, public SSLDecoder
{
public:
	/// Object constructor
	SSLWrapper_Analyzer(RSA * arg_rsa, ContentLine_Analyzer * orig, ContentLine_Analyzer * resp);
	/// Objet destructor
	~SSLWrapper_Analyzer();
	/// This function is called from the network layer when we have reassembled packets
	virtual void DeliverStream(int len, const unsigned char * data, bool is_orig);
	/// This function is called from the network layer when we have missing packets. In case this happens we are not able to decrypt this connection any more.
	virtual void Undelivered(int seq, int len, bool orig);
	/// This function is called from the SSLDecoder when we have decrypted data
	virtual void DeliverClear(int len, const unsigned char* data, bool is_orig);
private:
	ContentLine_Analyzer * content_line_orig;
	ContentLine_Analyzer * content_line_resp;
	std::string orig_msg;
	std::string resp_msg;
};

#endif
