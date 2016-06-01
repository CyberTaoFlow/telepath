#include <string>
#include <map>
#include <utility>
#include <string.h>

#include "ssl_sessions.h"

/**
 * The SSLSession structure is used to store so called SSL sessions.
 * They are used to quickly reopen SSL connections.
 * In case of SSL sessions, limited version of SSL handshake is performed.
 * Same structure is used for SSL sessions and for SSL tickets.
 */
struct SSLSession
{
	///  When new SSL connection is reopened, it will used this master key.
	unsigned char master_key[48];
	/// compression method specifies if reopen SSL connection should use compression
	unsigned int compression;
};

/**
 * ssl_sessions map stores all SSL sessions that can be reopened.
 */
static std::map<std::string, class SSLSession,  std::less< std::string > > ssl_sessions;

/**
 * addSession() function adds a new session to the session cache.
 *
 * @param key session key to use
 * @param master_key pints to a master key to save in cache
 */
bool addSession(const unsigned char *key, const unsigned char * master_key)
{
	//mymap.insert ( std::pair<char,int>('a',100) );
	std::string session_key;
	SSLSession session;

	session_key.assign((char*)key, 32);
	memcpy(&session.master_key, master_key, 48);
	session.compression = 0;

	ssl_sessions[session_key] = session;
	//ssl_sessions.insert( std::pair<std::string,SSLSession>(session_key,session) );
	return true;
}

/**
 * findSession() function performs session lookup using key provided.
 *
 * @param key points to the session key to search
 * @param master_key poins to the string where to copy master key if it is found.
 * @return true if session was found. master_key points to a master key to use.
 */
bool findSession(const unsigned char * key, unsigned char * master_key)
{
	std::map<std::string, SSLSession>::iterator itr;
	std::string session_key;

	session_key.assign((char*)key, 32);
	itr = ssl_sessions.find(session_key);
	if (itr != ssl_sessions.end())
	{
		memcpy(master_key, &itr->second.master_key, 48);
		return true;
	}
	
	return false;
}

/**
 * addSessionTicket() function adds a new session ticket to the session cache.
 *
 * @param key session key to use
 * @param master_key pints to a master key to save in cache
 * @param compression compression method to use
 */

bool addSessionTicket(const std::string & key, const unsigned char * master_key,  const unsigned int compression)
{
	//mymap.insert ( std::pair<char,int>('a',100) );
	SSLSession session;

	memcpy(&session.master_key, master_key, 48);
	session.compression = compression;

	ssl_sessions[key] = session;
	//ssl_sessions.insert( std::pair<std::string,SSLSession>(session_key,session) );
	return true;
}

/**
 * findSessionTicket() function performs session lookup using key provided.
 *
 * @param key points to the session key to search
 * @param master_key poins to the string where to copy master key if it is found
 * @param compression method reference
 * @return true if session was found. master_key points to a master key to use.
 */
bool findSessionTicket(const std::string & key, unsigned char * master_key, unsigned int & compression)
{
	std::map<std::string, SSLSession>::iterator itr;

	itr = ssl_sessions.find(key);
	if (itr != ssl_sessions.end())
	{
		memcpy(master_key, &itr->second.master_key, 48);
		compression = itr->second.compression;
		return true;
	}
	
	return false;
}

/**
 * removeOldSessions() function should remove old sessions from session cache map. Currently it is not implremented.
 */
bool removeOldSessions()
{
	return true;
}
