#ifndef analyzer_h
#define analyzer_h

class Analyzer
{
public:
	virtual void DeliverStream(int len, const unsigned char* data, bool is_orig) = 0;
	virtual void Undelivered(int seq, int len, bool is_orig) = 0;
};

#endif