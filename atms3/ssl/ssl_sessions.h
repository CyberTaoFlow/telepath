#ifndef _SSL_SESSION_HPP_
#define _SSL_SESSION_HPP_ 1

#include <string>

/// This function adds a new session to the session cache.
bool addSession(const unsigned char *_key, const unsigned char * master_key);
/// This function performs session lookup using key provided.
bool findSession(const unsigned char * _key, unsigned char * master_key);
/// This function should remove old sessions from session cache map. Currently it is not implremented.
bool removeOldSessions();
/// This function adds a new session ticket to the session cache.
bool addSessionTicket(const std::string & key, const unsigned char * master_key, const unsigned int compression);
///  This function performs session lookup using key provided
bool findSessionTicket(const std::string & key, unsigned char * master_key, unsigned int & compression);

#endif
