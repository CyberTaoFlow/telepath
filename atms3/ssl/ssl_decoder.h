#ifndef _SSL_DECODER_HPP_
#define _SSL_DECODER_HPP_ 1

#include <string>
#include <time.h>
#include <openssl/ssl.h>
#include <openssl/rc4.h>
#include <openssl/aes.h>

#include "zlib.h"

/*
 3,0 - SSL3
 3,1 - TLS1.0
 3,2 - TLS1.1
 3.3 - TLS1.2
 */

// supported ciphers
#define SSL_RC4_128_MD5 4
#define SSL_RC4_128_SHA1 5
#define SSL_AES_128_SHA1 0x2F  // curl -k --ciphers "AES" https://my.bezeq.co.il -d "test=1"
#define SSL_AES_256_SHA1 0x35  // curl -k --ciphers "AES256-SHA" https://my.bezeq.co.il -d "test=1"

/// SSL connection state enum
enum ssl_state {ssl_client_hello = 1, ssl_server_hello = 2, ssl_server_certificate=11, ssl_server_hello_done = 14, 
	ssl_client_key_exchange=16, ssl_client_change_cipher=20, ssl_client_finished,
	ssl_server_change_cipher, ssl_server_finished, ssl_data, ssl_error =100};

/**
 * SSLDecoder is a main SSL decoding class.
 *
 */
class SSLDecoder
{
public:
	/// Object constructor.
	SSLDecoder(RSA *_rsa): rsa(_rsa), state(ssl_client_hello),client_protocol(0),client_session_len(0),client_session_reuse_state(ssl_client_hello), server_session_len(0), server_session_reuse_state(ssl_server_hello),cipher(0),write_key_size(0),hash_size(0),iv_size(0),is_exportable(0),block_size(0),compression_protocol(0), session_reuse(0), keys_generated(0) {}
	/// This function is used to decrypt a number of client packets.
	int ParseClientPackets(const unsigned char * date, const size_t len);
	/// Handle single SSL packet. This function is used during SSL handshake.
	int ParseClientSinglePacket(const unsigned char * date, size_t len);
	/// Decodes a number of server packets. On each new packet ParseServerSinglePacket() function is called
	int ParseServerPackets(const unsigned char * data, const int len);
	/// Parse single server packet. This function is used during handshake to establish SSL connection.
	int ParseServerSinglePacket(const unsigned char * data, int len);
	/// This function is used to check if we received the whole SSL packet.
	int CheckIfReady(const unsigned char * data, const size_t len);
	/// This function returns true if we are able to process this SSL session or false in case of an error.
	bool IsGood() { return state != ssl_error; }
	/// This function marks this SSL session as broken.
	void SetError() { state = ssl_error; }
	/// This function actually transmeats decrypted packet (plaintext) back to BRO Http_Analyzer class.
	virtual void DeliverClear(int len, const unsigned char* data, bool is_orig) = 0;
private:
	void HMAC_MD5_Prepare(const unsigned char * key, unsigned int key_len, unsigned char * o_key, unsigned char * i_key );
	void HMAC_SHA_Prepare(const unsigned char * key, unsigned int key_len, unsigned char * o_key, unsigned char * i_key );
	/// This function is used to generate master key for TLS1.0 and TLS1.1 out of pre master key received and client and server random bytes.
	ssl_state TLS1GenerateMasterkey(const unsigned char * pre_master_key, const int key_len);
	/// This function is used to generate MAC, keys and IV out of master key used for TLS1.0 and TLS1.1 .
	bool TLS1GenerateKeys();
	/// This function is used to generate master key used in SSLv3.
	ssl_state SSL3GenerateMasterkey(const unsigned char * pre_master_key, const int key_len);
	/// This function is actually not used. 
	bool SSL3ValidateFinished(bool from_client, const unsigned char * keys, int key_len);
	/// This function is used to generate MAC, keys and IV out of master key used for SSLv3.
	bool SSL3GenerateKeys();
	/// Final packet decryption. Decrypt single SSL client packet.
	int DecryptClientPacket(const unsigned char * data, const int len);
	/// Final packet decryption. Decrypt single SSL server packet.
	int DecryptServerPacket(const unsigned char * data, const int len);
	/// This function is used to generate "preudorandom" bytes used in TLS1.0 and TLS1.1 .
	void PRF(const unsigned char * key, unsigned int key_len, 
		const unsigned char * seed, const int seed_length,
		unsigned char * result, const size_t result_size);
	/// Decompress compressed packet. Using zlib for that.
	bool ZLIBDecompress(z_stream & strm, const unsigned char * from, unsigned int size,std::string & result);
	/// RSA structure is used to store private ke used in this connection.
	RSA *rsa;
	/// SSL Connection state. For example ssl_handshake or ssl_error
	ssl_state state;
	/* 03 02 -> tls 1.1
	 * 03 03 -> sslv_23
	 */
	/// Client protocol as seen in first user SSL handshake packet.
	unsigned int client_protocol;
	/// Client time as transmeted by client. Actually not used. It is just printed in debug mode.
	time_t client_time;
	/// Client session ticket. it is used to perform quick SSL reconection.
	std::string client_ticket;
	/// Stream of random bytes send by cleints. Used during master key generation.
	unsigned char client_random[28+4];
	/// client_session is used when client wants to reuse establish SSL session.
	unsigned char client_session[32];
	/// client_session length
	unsigned int client_session_len;
	unsigned char client_md5[16];
	unsigned char client_sha1[20];
	unsigned char client_write[100];
	/// client_write_mac_secret can be used to 
	unsigned char client_write_mac_secret[100];
	/// client_write_iv is used for AES encryption. IV or Initial Vector is used to make encryption more secure. It is used to intermix ciphertext with previous encryption state.
	unsigned char client_write_iv[100];
	ssl_state client_session_reuse_state;
	/// Client zlib compression structure. It is important to keep it in class and not to reset it on each new SSL packet. Otherwise only first packet will be decompressed.
	z_stream client_strm;
	RC4_KEY rc4_client_key;
	AES_KEY aes_client_key;

	unsigned int server_protocol;
	/// Server time as transmeted by server. Actually not used. It is just printed in debug mode.
	time_t server_time;
	std::string server_ticket;
	/// Stream of random bytes send by server. Used during master key generation.
	unsigned char server_random[28+4];
	/// Server sends server_sessin when SSL connection is almost established. This SSL session can be used on next client connection to quickly recover master key and to perform quick SSL handshake.
	unsigned char server_session[32];
	/// server_session length.
	unsigned int server_session_len;
	unsigned char server_write[100];
	unsigned char server_write_mac_secret[100];
	/// server_write_iv is used for AES encryption. IV or Initial Vector is used to make encryption more secure. It is used to intermix ciphertext with previous encryption state.
	unsigned char server_write_iv[100];
	ssl_state server_session_reuse_state;
	/// Server zlib compression structure. It is important to keep it in class and not to reset it on each new SSL packet. Otherwise only first packet will be decompress.
	z_stream server_strm;
	RC4_KEY rc4_server_key;
	AES_KEY aes_server_key;

	unsigned char pre_master_key[48];
	/// Main SSL secret. Connection master key.
	unsigned char master_key[48];
	std::string handshake_msg;
	unsigned int cipher;

	unsigned int write_key_size;
	/// Connection hash size. It can be 16 for MD5 or 20 for SHA1
	unsigned int hash_size;
	/// IV or Initial Vecotr size.
	unsigned int iv_size;
	/// Exportable encryption flag.
	unsigned int is_exportable;
	unsigned int block_size;
	/// Compression method. If 0 compression not used. In case of 1 - zlib compression is used.
	unsigned int compression_protocol;
	int session_reuse;
	int keys_generated;
};


#endif
