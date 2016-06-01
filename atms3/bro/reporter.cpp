//
// See the file "COPYING" in the main distribution directory for copyright.
//

//#define WAIT_FOR_CONNECT 1
//#define WRITE_DUMP "c:\\cert\\mem-dump.txt"
//#define OVERRIDE_NEW_DELETE
//#include "MemProCpp\MemPro.cpp"

//#include <syslog.h>

#include "config.h"
#include "reporter.h"
//#include "Event.h"
//#include "NetVar.h"
//#include "Net.h"
//#include "Conn.h"

#ifdef SYSLOG_INT
extern "C" {
int openlog(const char* ident, int logopt, int facility);
int syslog(int priority, const char* message_fmt, ...);
int closelog();
}
#endif

Reporter* reporter = 0;

Reporter::Reporter()
	{
	errors = 0;
	via_events = false;
	in_error_handler = 0;

	//openlog("bro", 0, LOG_LOCAL5);
	}

Reporter::~Reporter()
	{
	//closelog();
	}

void Reporter::Info(const char* fmt, ...)
	{
	va_list ap;
	va_start(ap, fmt);
	DoLog("", stderr, fmt, ap);

	va_end(ap);
	}

void Reporter::Warning(const char* fmt, ...)
	{
	va_list ap;
	va_start(ap, fmt);
	DoLog("warning", stderr, fmt, ap);
	va_end(ap);
	}

void Reporter::Error(const char* fmt, ...)
	{
	++errors;
	va_list ap;
	va_start(ap, fmt);
	DoLog("error", stderr, fmt, ap);
	va_end(ap);
	}

void Reporter::FatalError(const char* fmt, ...)
	{
	va_list ap;
	va_start(ap, fmt);

	// Always log to stderr.
	DoLog("fatal error", stderr, fmt, ap);

	va_end(ap);

	set_processing_status("TERMINATED", "fatal_error");
	exit(1);
	}

void Reporter::FatalErrorWithCore(const char* fmt, ...)
	{
	va_list ap;
	va_start(ap, fmt);

	// Always log to stderr.
	DoLog("fatal error", stderr, fmt, ap);

	va_end(ap);

	set_processing_status("TERMINATED", "fatal_error");
	abort();
	}
void Reporter::InternalError(const char* fmt, ...)
	{
	va_list ap;
	va_start(ap, fmt);

	// Always log to stderr.
	DoLog("internal error", stderr, fmt, ap);

	va_end(ap);

	//set_processing_status("TERMINATED", "internal_error");
	//abort();
	}

void Reporter::InternalWarning(const char* fmt, ...)
	{
	va_list ap;
	va_start(ap, fmt);
	DoLog("internal warning", stderr, fmt, ap);
	va_end(ap);
	}

void Reporter::Syslog(const char* fmt, ...)
	{
	//if ( reading_traces )
	//	return;

	va_list ap;
	va_start(ap, fmt);
	DoLog("syslog", stderr, fmt, ap);
	//vsyslog(LOG_NOTICE, fmt, ap);
	va_end(ap);
	}

void Reporter::Weird(const char* name)
	{
		fprintf(stdout, "weird: %s\n", name);
	//WeirdHelper(net_weird, 0, 0, name);
	}

void Reporter::Weird(const IPAddr& orig, const IPAddr& resp, const char* name)
	{
	//WeirdFlowHelper(orig, resp, "%s", name);
		fprintf(stdout, "weird: %s: orig: %s, dest: %s\n", name, orig.AsString().c_str(), resp.AsString().c_str());
	}

void Reporter::Weird(const char* name, const class IP_Hdr* ip)
	{
		Weird(ip->SrcAddr(), ip->DstAddr(), name);
	}

void Reporter::DoLog(const char* prefix, FILE* out, const char* fmt, va_list ap)
	{
	fprintf(out, "%s: ", prefix);
	vfprintf(out, fmt, ap);
	fprintf(out, "\n");
	}

