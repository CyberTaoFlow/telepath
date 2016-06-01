#include "config.h"
#include "contentline.h"
#include "ssl_wrapper.h"
#include "util.h"

SSLWrapper_Analyzer::SSLWrapper_Analyzer(RSA * arg_rsa, ContentLine_Analyzer * orig, ContentLine_Analyzer * resp):
SSLDecoder(arg_rsa)
{
	content_line_orig = orig;
	content_line_resp = resp;
}

SSLWrapper_Analyzer::~SSLWrapper_Analyzer()
{
}

void SSLWrapper_Analyzer::DeliverStream(int len, const unsigned char* data, bool is_orig)
{
	if (!IsGood())
	{
		DEBUG_MSG("Skipping broken SSL connection.\n");
		return;
	}
	if (is_orig)
	{
		if (orig_msg.size() == 0)
		{
			if (CheckIfReady( data, len) == -1)
			{
				if (!IsGood())
        			{
					DEBUG_MSG("Skipping broken SSL connection2.\n");
					return;
				}
				// need to waite for more data
				orig_msg.append((char*)data, len);
				return;
			}
			ParseClientPackets( data, (size_t) len);
			return;
		} else {
			// append current message to orig_msg
			orig_msg.append((char*)data, len);
			if (CheckIfReady( (unsigned char *) orig_msg.c_str(), orig_msg.size()) == -1)
			{
				// need to wait for more data
				return;
			}
			ParseClientPackets( (unsigned char*) orig_msg.c_str(), orig_msg.size());
			orig_msg.clear();
		}
	} else {
		if (resp_msg.size() == 0)
		{
			if (CheckIfReady( data, len) == -1)
			{
				if (!IsGood())
				{
					DEBUG_MSG("Skipping broken SSL connection3.\n");
					return;
				}
				// need to waite for more data
				resp_msg.append((char*)data, len);
				return;
			}
			ParseServerPackets( data, (size_t) len);
			return;
		} else {
			// append current message to orig_msg
			resp_msg.append((char*)data, len);
			if (CheckIfReady( (unsigned char *) resp_msg.c_str(), resp_msg.size()) == -1)
			{
				// need to wait for more data
				return;
			}
			ParseServerPackets( (unsigned char*) resp_msg.c_str(), resp_msg.size());
			resp_msg.clear();
		}
	}
}

void SSLWrapper_Analyzer::Undelivered(int seq, int len, bool orig)
{
	// we have undelived data
	// no need to do anything here
	// the ssl connection can not continue if one of the packets is missing
	SetError();
	orig_msg.clear();
	resp_msg.clear();
}

void SSLWrapper_Analyzer::DeliverClear(int len, const unsigned char* data, bool is_orig)
{
	if (is_orig)
	{
		content_line_orig->DeliverStream(len, data, true);
	} else {
		content_line_resp->DeliverStream(len, data, false);
	}
}
