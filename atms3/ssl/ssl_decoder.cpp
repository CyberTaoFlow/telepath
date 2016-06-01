#include "ssl_decoder.h"

#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#include "utils.h"
#include "ssl_sessions.h"

const unsigned int md5_block_size = 64;


const static char month_str[][4] = { {"Jan"}, {"Feb"}, {"Mar"}, {"Apr"}, {"May"}, {"Jun"}, {"Jul"}, {"Aug"}, {"Sep"}, {"Oct"}, {"Nov"}, {"Dec"}, {""} };


/**
 * CheckIfReady() function is used to check if we received the whole SSL packet.
 *
 * @param data points to the packets.
 * @param len is a data length.
 * @return -1 if more data is expected to parse all packets in data.
 */
int SSLDecoder::CheckIfReady(const unsigned char * data, const size_t len)
{
	size_t start = 0;
	size_t pkt_size = 0;
	unsigned int first;

	if (len)
	{
		DEBUG_SSL_HEX(data, (len > 16) ? 16: len);
	}

        if (len < 5)
                return -1;

	do
	{
		first =  data[start];
		if (first == 0x16 || first == 0x14)
		{
			// do we have space to read header ?
			if (len - start < 5)
				return -1;
			pkt_size = data[start+3] << 8 | data[start+4];
			//DEBUG_SSL_MSG("handshake packet, expected size: %d, received %d\n", pkt_size, len);
			if (pkt_size+5 > len-start)
				return -1;
			start += pkt_size+5;
		} else if (first == 0x17 )
		{
	        	if (keys_generated == 0)
        		{
				DEBUG_SSL_MSG("We got SSL clint data without established SSL session\n");
				state = ssl_error;
				return -1;
			}
			pkt_size = data[start+3] << 8 | data[start+4];
			if (pkt_size+5 > len-start)
				return -1;
			start += pkt_size+5;
		} else {
			DEBUG_SSL_MSG("Check size: wrong first byte: %d\n", first);
			return 10;
		}
	} while ( start < len );

	return 1;
}

/**
 * This function is used to decrypt a number of client packets.
 * On each SSL packet we run ParseClientSinglePacket() function.
 *
 * @param data points to the packets.
 * @param len is a data length.
 */
int SSLDecoder::ParseClientPackets(const unsigned char * data, const size_t len)
{
	size_t start = 0;
	size_t pkt_size = 0;
	size_t first;

	if (len < 5)
		return -1;
	// if error - nothing to do - just push everything we got !
	if (state == ssl_error)
		return len;

	do
	{
		first = data[start];
		if (first == 0x16 || first == 0x14)
		{
			pkt_size = data[start+3] << 8 | data[start+4];
			//DEBUG_SSL_MSG("handshake packet, expected size: %d, received %d\n", pkt_size, len);
			if (pkt_size+5 > len-start)
				return -1;
			ParseClientSinglePacket(data+start, pkt_size+5);
			if (state == ssl_error)
				return -1;
			start += pkt_size+5;
		} else if (first == 0x17 )
		{
			pkt_size = data[start+3] << 8 | data[start+4];
			DecryptClientPacket(data+start, pkt_size);
			if (state == ssl_error)
				return -1;
			start += pkt_size+5;
		} else {
			return 10;
		}
	} while ( start < len );

	return 1;
}

/**
 * Final packet decryption. Decrypt single SSL client packet.
 *
 * @param data points to the client packet.
 * @param len is a data length.
 */
int SSLDecoder::DecryptClientPacket(const unsigned char * data, const int len)
{
	unsigned char md5_signature[16];
	unsigned int pad = 0;
	// sanity check
        if (keys_generated == 0)
        {
                DEBUG_SSL_MSG("We got SSL clint data without established SSL session\n");
                state = ssl_error;
                return -1;
        }

	unsigned char result[len+100];
	DEBUG_SSL_MSG("encrypted data\n");

	// decrypt packet contents
	switch(cipher)
	{
		case SSL_RC4_128_MD5:
		case SSL_RC4_128_SHA1:
			RC4(&rc4_client_key, len, data+5, (unsigned char*)result);
			break;
		case SSL_AES_128_SHA1:
		case SSL_AES_256_SHA1:
			AES_cbc_encrypt(data+5, (unsigned char*)result, len, &aes_client_key, client_write_iv, AES_DECRYPT);
			if (len > 0)
				pad = result[len-1]+1;
			break;
	}

	// decompressing packet contents
	if (compression_protocol == 1)
	{
		std::string str;
		ZLIBDecompress(client_strm, result, len-hash_size-pad, str);
		DEBUG_SSL_MSG("client:\n%s", str.c_str());
		DeliverClear(str.size(), (unsigned char *)str.c_str(), true);
		return len;
	}
	DeliverClear(len-hash_size-pad, result, true);

	return len;
	//if (compression_protocol == 1)
	//{
	//	ZLIBDecompress(result, len, str);
	//}

	std::string str;
	str.append((char*)result, len-hash_size);
	DEBUG_SSL_MSG("client:\n%s", str.c_str());

	if (hash_size == 16)
		MD5(result, len-hash_size, md5_signature);

	return len;
}

/**
 * Final packet decryption. Decrypt single SSL server packet.
 *
 * @param data points to the server packet.
 * @param len is a data length.
 */
int SSLDecoder::DecryptServerPacket(const unsigned char * data, const int len)
{
	unsigned char md5_signature[16];
	unsigned char result[len+100];
	unsigned int pad = 0;
	// sanity check
        if (keys_generated == 0)
        {
                DEBUG_SSL_MSG("We got ssl response without established SSL session\n");
                state = ssl_error;
                return -1;
        }
	DEBUG_SSL_MSG("encrypted server data\n");

	// decrypt packet
	switch(cipher)
	{
		case SSL_RC4_128_MD5:
		case SSL_RC4_128_SHA1:
			RC4(&rc4_server_key, len, data+5, (unsigned char*)result);
			break;
		case SSL_AES_128_SHA1:
		case SSL_AES_256_SHA1:
			AES_cbc_encrypt(data+5, (unsigned char*)result, len, &aes_server_key, server_write_iv, AES_DECRYPT);
			if (len > 0)
				pad = result[len-1]+1;
			break;
	}
	// decompress packet if required
	if (compression_protocol == 1)
	{
		std::string str;
		ZLIBDecompress(server_strm, result, len-hash_size-pad, str);
		DEBUG_SSL_MSG("server:\n%s", str.c_str());
		DeliverClear(str.size(), (unsigned char *)str.c_str(), false);
		return len;
	}
	DeliverClear(len-hash_size-pad, result, false);
	return len;

	std::string str;
	str.append((char*)result, len-hash_size);
	DEBUG_SSL_MSG("server:\n%s", str.c_str());

	if (hash_size == 16)
		MD5(result, len-hash_size, md5_signature);

	return len;
}

/**
 * Handle single SSL packet. This function is used during SSL handshake.
 *
 */
int SSLDecoder::ParseClientSinglePacket(const unsigned char * pkt_data, size_t len)
{
	size_t first = pkt_data[0];
	size_t pkt_size = pkt_data[3] << 8 | pkt_data[4];
	size_t header_size = pkt_data[6] << 16 | pkt_data[7] << 8 | pkt_data[8];
	size_t pkt_protocol = pkt_data[1] << 8 | pkt_data[2];
	size_t pos = 0;
	const unsigned char * pkt = pkt_data+5;

	DEBUG_SSL_MSG("client single packet, first " SIZET_FMT ", expected %d got %d, size " SIZET_FMT "\n", first, state, *pkt, pkt_size);
	if (first != 0x16 && first != 0x14)
	{
		return 0;
	}

	if (first == 0x16 && state != ssl_client_finished) // ssl_client_change_cipher not used !
	{
		handshake_msg.append( (char*)pkt,pkt_size);
		if (header_size+4 != pkt_size)
		{
			DEBUG_SSL_MSG("ParseClientSinglePacket wrong size\n");
			state = ssl_error;
			return 0;
		}
	}
	if (state == ssl_client_key_exchange)
	{
		//This message contains what SSL calls the PreMasterSecret encrypted with the server's public key that was in its certificate.
		if (state != *pkt)
		{
			DEBUG_SSL_MSG("wrong state: %d\n", pkt[4]);
			return 0;
		}
		// in old SSL protocol there is no additional size packet bellow !!!
		// The message length field specifies 128 bytes. The enlargement from 48 to 128 bytes is the result of the RSA encryption.
		// In TLS, the EncryptedPreMasterSecret is defined as a variable array of between 0 and 216 1 bytes and therefore has a 2-byte length prefix.
		//
		size_t enc_size = pkt_size - 2;
		const unsigned char * start = pkt+1;
		if ( pkt_protocol == 0x300)
		{
			enc_size = pkt[1] << 16 | pkt[2] << 8 | pkt[3];
			start = pkt+4;
		} else if ( pkt_protocol == 0x301)
		{
			enc_size = pkt[1] << 16 | pkt[2] << 8 | pkt[3];
			enc_size-=2;
			start = pkt+6;
		} else if ( pkt_protocol >= 0x302)
		{
			enc_size = pkt[1] << 16 | pkt[2] << 8 | pkt[3];
			enc_size-=2;
			start = pkt+6;
		}
		DEBUG_SSL_MSG("encrypted pre master key size: " SIZET_FMT ", protocol " SIZET_FMT "\n", enc_size, pkt_protocol);
		DEBUG_SSL_HEX(start, enc_size);

		unsigned char pre[10240];
		int decrypted = RSA_private_decrypt(enc_size, start, (unsigned char*) pre, rsa, RSA_PKCS1_PADDING );
		if (decrypted <= 0)
		{
			DEBUG_SSL_MSG("Failed to decrypt pre master key using private key !\n");
			state = ssl_error;
			return -1;
		}
		if (decrypted != 48)
		{
			DEBUG_SSL_MSG("Wrong decrypted pre master key size !!!!\n");
			state = ssl_error;
			return -1;
		}
		DEBUG_SSL_MSG("decrypted: %d\n", decrypted);
		DEBUG_SSL_HEX(pre, decrypted);
		memcpy(pre_master_key, pre, 48);

		//For all key exchange methods, the same algorithm is used to convert
		//the pre_master_secret into the master_secret. (from RSA)

		if (pkt_protocol == 0x300)
		{
			SSL3GenerateMasterkey(pre_master_key, decrypted);
		} else if (pkt_protocol == 0x301)
		{
			TLS1GenerateMasterkey(pre_master_key, decrypted);
		}
		else if (pkt_protocol == 0x302)
		{
			// same as in TLS1.0
			TLS1GenerateMasterkey(pre_master_key, decrypted);			
		} else {
			DEBUG_SSL_MSG("Unsupported protocol\n");
			state = ssl_error;
			return 0;
		}
		if (server_session_len != 0)
		{
			addSession(server_session, master_key);
		}
		state = ssl_client_change_cipher;
		DEBUG_SSL_MSG("session master key:\n");
		DEBUG_SSL_HEX(master_key, sizeof(master_key));
	} else if ((state == ssl_client_change_cipher || client_session_reuse_state == ssl_client_change_cipher) && 
		first == 0x14)
	{
		if (0x1 != *pkt)
		{
			DEBUG_SSL_MSG("wrong state: %d\n", pkt[4]);
			state = ssl_error;
			return 0;
		}
		state = ssl_client_finished;
		client_session_reuse_state = ssl_client_finished;
	} else if (state == ssl_client_finished || client_session_reuse_state == ssl_client_finished)
	{
		unsigned char result[10240];
		DEBUG_SSL_MSG("encrypted handshake message / finish message, size " SIZET_FMT "\n", pkt_size);
		if (keys_generated == 0)
		{
			if (pkt_protocol == 0x300)
			{
				SSL3GenerateKeys();
			} else if (pkt_protocol == 0x301)
			{
				TLS1GenerateKeys();
			} else if (pkt_protocol == 0x302)
			{
				TLS1GenerateKeys();
			}
			keys_generated = 1;
			if (session_reuse)
			{
				state = server_session_reuse_state;
			} else {
				state = ssl_server_change_cipher;
			}
		} else {
			state = ssl_data;
		}

                switch(cipher)
                {
			case SSL_RC4_128_MD5:
			case SSL_RC4_128_SHA1:
				RC4_set_key(&rc4_client_key, write_key_size, client_write);
				RC4(&rc4_client_key, pkt_size, pkt, (unsigned char*)result);
				break;
			case SSL_AES_128_SHA1:
				AES_set_decrypt_key(client_write, 128, &aes_client_key);
				AES_cbc_encrypt(pkt, (unsigned char*)result, pkt_size, &aes_client_key, client_write_iv, AES_DECRYPT);
				break;
			case SSL_AES_256_SHA1:
				AES_set_decrypt_key(client_write, 256, &aes_client_key);
				AES_cbc_encrypt(pkt, (unsigned char*)result, pkt_size, &aes_client_key, client_write_iv, AES_DECRYPT);
				break;
		}
		
		if (compression_protocol== 1)
		{
			unsigned int pad = 0;
			switch(cipher)
			{
				case SSL_AES_128_SHA1:
				case SSL_AES_256_SHA1:
					pad = result[pkt_size-1]+1;
					break;
			}
			std:: string str;
			// works when pkt_size = 0x14
			ZLIBDecompress(client_strm, result, pkt_size-hash_size-pad, str);
			//	memcpy(result, str.c_str(), str.size());
		}

		memcpy(client_md5, result+4, sizeof(client_md5));
		DEBUG_SSL_MSG("Client MD5:\n");
		DEBUG_SSL_HEX(client_md5, sizeof(client_md5));
		
		memcpy(client_sha1, result+4+sizeof(client_md5), sizeof(client_sha1));
		DEBUG_SSL_MSG("Client SHA1:\n");
		DEBUG_SSL_HEX(client_sha1, sizeof(client_sha1));

		/*
		    enum { client(0x434C4E54), server(0x53525652) } Sender;
		    struct {
			    opaque md5_hash[16];
				opaque sha_hash[20];
			} Finished;
			md5_hash:  MD5(master_secret + pad2 + MD5(handshake_messages + Sender + master_secret + pad1));
			sha_hash:  SHA(master_secret + pad2 + SHA(handshake_messages + Sender + master_secret + pad1));

			handshake_messages:  All of the data from all handshake messages up
			to but not including this message.  This is only data visible at
	        the handshake layer and does not include record layer headers.
		
			Note: Change cipher spec messages are not handshake messages and are not included in the hash computations.
							
			struct {		          opaque verify_data[verify_data_length];
			} Finished;

			verify_data
				PRF(master_secret, finished_label, Hash(handshake_messages))
				[0..verify_data_length-1];

			finished_label
				ForFinished messages sent by the client, the string
				"client finished".  For Finished messages sent by the server,
				the string "server finished".

		*/
		if (pkt_protocol == 0x300)
		{
			//SSL3ValidateFinished(true, result+4, 1);
		}

	// first packet in SSL handshake
	} else if (state == ssl_client_hello)
	{
		if (state != *pkt)
		{
			DEBUG_SSL_MSG("wrong state: %d\n", pkt[4]);
			state = ssl_error;
			return 0;
		}
		size_t hello_size = pkt[1] << 16 | pkt[2] << 8 | pkt[3];
		DEBUG_SSL_MSG("hello size: " SIZET_FMT "\n", hello_size);

		if (hello_size < 2)
			return 0;

		client_protocol = pkt[4] << 8 | pkt[5];
		hello_size -= 2;

		pos = 6;
		client_time = (unsigned int) (pkt[6] << 24 | pkt[7] << 16 | pkt[8] << 8 | pkt[9]);
		struct tm _timeinfo;
		struct tm * timeinfo = &_timeinfo;
		timeinfo = localtime(&client_time);
		DEBUG_SSL_MSG("client time: [%02d/%s/%02d %d:%02d:%02d]\n", timeinfo->tm_mday, month_str[timeinfo->tm_mon], timeinfo->tm_year+1900,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); 

		memcpy(client_random, pkt+pos, 32);
		DEBUG_SSL_MSG("random sesion[28]\n");
		DEBUG_SSL_HEX(client_random+4, sizeof(client_random)-4);
		hello_size -= 32;
		pos += 32;

		if (hello_size < 1)
			return 0;
		client_session_len = pkt[pos++];
		hello_size--;
		if (client_session_len > 0 && client_session_len != 32)
		{
			DEBUG_SSL_MSG("broken session len value\n");
			return 0;
		}
		if (client_session_len != 0 && client_session_len <= hello_size)
		{
		  DEBUG_SSL_MSG("we have a client session request.\n");
		  memcpy(client_session, pkt+pos, client_session_len);
		  pos += client_session_len;
		  hello_size -= client_session_len;
		}

		if (hello_size < 2)
			return 0;
		size_t c_length = pkt[pos] << 8 | pkt[pos+1];
		DEBUG_SSL_MSG("number of supported ciphers: " SIZET_FMT "\n", c_length/2);
		pos+=2;
		hello_size -=2;
		
		if (c_length > hello_size )
		{
			//broken packet!
			return 0;
		}
		//DEBUG_SSL_HEX(pkt+pos, c_length);
		pos += c_length;
		hello_size -= c_length;
		
		if (hello_size == 0)
			return 0;

		size_t compression_length = pkt[pos++];
		hello_size--;
		if (compression_length > hello_size)
			return 0;
		if (compression_length > 1)
			DEBUG_SSL_MSG("compression: %s\n", pkt+pos);
		pos += compression_length;
		hello_size -= compression_length;
		
		state = ssl_server_hello;
		if (hello_size < 2)
			return 1; // we are ok if we got here
		
		size_t all_ext_length = pkt[pos] << 8 | pkt[pos+1];
		size_t ext_type = 0;
		size_t ext_length = 0;
		hello_size -=2;
		pos+=2;
		if (all_ext_length > hello_size)
		{
			DEBUG_SSL_MSG("ssl broken, no place for extensions! " SIZET_FMT " - " SIZET_FMT "\n", all_ext_length, hello_size);
			return 0;
		}
		//DEBUG_SSL_MSG("ssl client extensions\n");
		int try_count = 10;
		while (hello_size >= 4 && try_count)
		{
			try_count--;
			ext_type = pkt[pos] << 8 | pkt[pos+1];
			pos+= 2;
			ext_length = pkt[pos] << 8 | pkt[pos+1];
			pos+= 2;
			hello_size-= 4;

			if (ext_length > hello_size)
				return 1;
			if (ext_type == 0x0)
			{
				// server name
				DEBUG_SSL_MSG("server name:\n");
				DEBUG_SSL_HEX(pkt+pos, ext_length);
			} else if (ext_type == 0x23)
			{
				if (ext_length > 0)
				{
					DEBUG_SSL_MSG("client session ticket: " SIZET_FMT "\n", ext_length);
					client_ticket.assign((char*) pkt+pos, ext_length);
					DEBUG_SSL_HEX(pkt+pos, ext_length);
				} else {
					DEBUG_SSL_MSG("client session ticket supported\n");
				}
			}
			hello_size -= ext_length;
			pos+=ext_length;
		}

	}

	return 1;
}


/**
 *
 * Decodes a number of server packets.
 * On each new packet ParseServerSinglePacket() function is called
 *
 */
int SSLDecoder::ParseServerPackets(const unsigned char * data, const int len)
{
	size_t start = 0;
	int pkt_size = 0;
	int first;
	if (len < 5)
		return -1;

	// if error - nothing to do - just push everything we got !
	if (state == ssl_error)
		return len;

	do
	{
		first =  data[start];
		if (first == 0x16 || first == 0x14)
		{
			pkt_size = data[start+3] << 8 | data[start+4];
			//DEBUG_SSL_MSG("handshake packet, expected size: %d, received %d\n", pkt_size, len);
			if (pkt_size+5 > len-start)
				return -1;
			ParseServerSinglePacket(data+start, pkt_size+5);
			if (state == ssl_error)
				return -1;
			start += pkt_size+5;
		} else if (first == 0x17)
		{
			pkt_size = data[start+3] << 8 | data[start+4];
			DecryptServerPacket(data+start, pkt_size);
			if (state == ssl_error)
				return -1;
			start += pkt_size+5;
		} else {
			return 10;
		}
	} while ( start < len );

	return 1;
}


/**
 * Parse single server packet. This function is used during handshake to establish SSL connection.
 *
 */
int SSLDecoder::ParseServerSinglePacket(const unsigned char * pkt_data, int len)
{
	unsigned int first = pkt_data[0];
	int pos = 0;
	int pkt_size = pkt_data[3] << 8 | pkt_data[4];
	int pkt_protocol = pkt_data[1] << 8 | pkt_data[2];
	const unsigned char * pkt = pkt_data+5;

	if (first != 0x14 && first != 0x16)
	{
		return 0;
	}

	DEBUG_SSL_MSG("server single packet, first %d, expected %d got %d, size %d\n", first, state, *pkt, pkt_size);

	if (first == 0x14)
	{
		if (state == ssl_server_change_cipher || server_session_reuse_state == ssl_server_change_cipher)
		{
			if (0x1 != *pkt)
			{
				DEBUG_SSL_MSG("ParseServerSinglePacket wrong state1: %d\n", pkt[4]);
				return 0;
			}
			state = ssl_server_finished;
			server_session_reuse_state = ssl_server_finished;
		} else {
			state = ssl_error;
		}
		return pkt_size+5;

	}

	if ( first == 0x16)
	{
		if (state == ssl_server_change_cipher || server_session_reuse_state == ssl_server_change_cipher)
		{
			server_ticket.assign((char*)pkt+10, pkt_size-10);
			addSessionTicket(server_ticket, master_key, compression_protocol);
			return pkt_size+5;
			/*
			unsigned char result[1024] = {0};
			unsigned char iv[16];
			AES_KEY key;

			//chrome cuts the first 10 bytes
			const unsigned char * ticket = pkt_data+10;
			// 128 is number of bits = iv sector size(16)*8
			AES_set_decrypt_key(ticket, 128, &key);
			//AES_decrypt(in, out, &k);
			memcpy(iv, ticket+16,16);
			AES_cbc_encrypt(ticket+32, result, pkt_size-10-32-30, &key, iv, AES_DECRYPT);

			DEBUG_SSL_MSG("server ticket,size %d\n", pkt_size);
			DEBUG_SSL_HEX(pkt, pkt_size);
			*/
		}
	}
	// if (first == 0x16)
	if (first == 0x16 && state < ssl_client_finished)
	{
		handshake_msg.append( (char*)pkt,pkt_size);
		int header_size = pkt[1] << 16 | pkt[2] << 8 | pkt[3];
		if (header_size+4 != pkt_size)
		{
			DEBUG_SSL_MSG("ParseServerSinglePacket wrong size2\n");
		}
	}

	if (state == ssl_server_finished || server_session_reuse_state == ssl_server_finished)
	{
		unsigned char result[10240];

		if (keys_generated == 0)
		{
			if (pkt_protocol == 0x300)
			{
				SSL3GenerateKeys();
			} else if (pkt_protocol == 0x301)
			{
				TLS1GenerateKeys();
			} else if (pkt_protocol == 0x302)
			{
				TLS1GenerateKeys();
			}
			keys_generated = 1;
			if (session_reuse)
				state = client_session_reuse_state;
		} else {
			state = ssl_data;
			server_session_reuse_state = ssl_data;
		}
                switch(cipher)
                {
			case SSL_RC4_128_MD5:
			case SSL_RC4_128_SHA1:
				RC4_set_key(&rc4_server_key, write_key_size, server_write);
				RC4(&rc4_server_key, pkt_size, pkt, (unsigned char*)result);
				break;
			case SSL_AES_128_SHA1:
				AES_set_decrypt_key(server_write, 128, &aes_server_key);
				AES_cbc_encrypt(pkt, (unsigned char*)result, pkt_size, &aes_server_key, server_write_iv, AES_DECRYPT);
				break;
			case SSL_AES_256_SHA1:
				AES_set_decrypt_key(server_write, 256, &aes_server_key);
				AES_cbc_encrypt(pkt, (unsigned char*)result, pkt_size, &aes_server_key, server_write_iv, AES_DECRYPT);
				break;
		}

		if (compression_protocol== 1)
		{
			unsigned int pad = 0;
			switch(cipher)
			{
				case SSL_AES_128_SHA1:
				case SSL_AES_256_SHA1:
					pad = result[pkt_size-1]+1;
				break;
			}
			std:: string str;
			// works when pkt_size = 0x14
			ZLIBDecompress(server_strm, result, pkt_size-hash_size-pad, str);
			//	memcpy(result, str.c_str(), str.size());
		}

		return pkt_size+5;
	} else if (state == ssl_server_certificate)
	{
		// we just need to forward this packet. No need for special handling
		if (state != *pkt)
		{
			DEBUG_SSL_MSG("wrong state: %d\n", pkt[4]);
			return 0;
		}
		state = ssl_server_hello_done;
		return pkt_size+5;
	} else if (state == ssl_server_hello_done)
	{
		//
		if (state != *pkt)
		{
			DEBUG_SSL_MSG("wrong state: %d\n", pkt[4]);
			return 0;
		}
		state = ssl_client_key_exchange;
		return pkt_size+5;
	} else if (state == ssl_server_hello)
	{
		if (state != *pkt)
		{
			DEBUG_SSL_MSG("wrong state: %d\n", pkt[4]);
			state = ssl_error;
			return 0;
		}
		size_t hello_size = pkt[1] << 16 | pkt[2] << 8 | pkt[3];
		size_t original_hello_size = hello_size;
		DEBUG_SSL_MSG("hello size: " SIZET_FMT "\n", hello_size);

		if (hello_size < 30)
		{
			state = ssl_error;
			return 0;
		}

		server_protocol = pkt[4] << 8 | pkt[5];
		if (server_protocol == 0x300)
		{
			DEBUG_SSL_MSG("Server is using protocol SSL3\n");
		} else if (server_protocol == 0x301)
		{
			DEBUG_SSL_MSG("Server is using protocol TLS1.0\n");
		} else if (server_protocol == 0x302)
		{
			DEBUG_SSL_MSG("Server is using protocol TLS1.1\n");
		} else {
			DEBUG_SSL_MSG("Server is using unknown protocol %d\n", server_protocol);
			state = ssl_error;
			return -1;
		}
		hello_size -= 2;

		pos = 6;
		server_time = (unsigned int) (pkt[6] << 24 | pkt[7] << 16 | pkt[8] << 8 | pkt[9]);
		struct tm _timeinfo;
		struct tm * timeinfo = &_timeinfo;
		timeinfo = localtime(&server_time);
		DEBUG_SSL_MSG("server time: [%02d/%s/%02d %d:%02d:%02d]\n", timeinfo->tm_mday, month_str[timeinfo->tm_mon], timeinfo->tm_year+1900,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); 

		memcpy(server_random, pkt+pos, 32);
		DEBUG_SSL_MSG("random sesion[28]:\n");
		DEBUG_SSL_HEX(server_random+4, sizeof(server_random)-4);
		hello_size -= 28;
		pos += 32;

		if (hello_size < 1)
		{
			state = ssl_error;
			return 0;
		}
		server_session_len = pkt[pos++];
		hello_size--;
		if (server_session_len > hello_size)
		{
			// broken session id
			state = ssl_error;
			return 0;
		}
		if (server_session_len == 32)
		{
			DEBUG_SSL_MSG("we have a server session !\n");
			memcpy(server_session, pkt+pos, server_session_len);
			pos+=server_session_len;
			hello_size -= server_session_len;
		}
		if (hello_size < 2)
		{
			state = ssl_error;
			return 0;
		}
		cipher = pkt[pos] << 8 | pkt[pos+1];
			
		/*
			+--------------+--------+-----+-------+-------+-------+------+------+
			| Cipher       | Cipher | IsE |  Key  |  Exp. | Effec |  IV  | Bloc |
			|              |  Type  | xpo | Mater |  Key  |  tive | Size |   k  |
			|              |        | rta |  ial  | Mater |  Key  |      | Size |
			|              |        | ble |       |  ial  |  Bits |      |      |
			+--------------+--------+-----+-------+-------+-------+------+------+
			| NULL         | Stream |  *  |   0   |   0   |   0   |   0  |  N/A |
			| FORTEZZA_CBC |  Block |     |   NA  |   12  |   96  |  20  |   8  |
			|              |        |     |  (**) |  (**) |  (**) | (**) |      |
			| IDEA_CBC     |  Block |     |   16  |   16  |  128  |   8  |   8  |
			| RC2_CBC_40   |  Block |  *  |   5   |   16  |   40  |   8  |   8  |
			| RC4_40       | Stream |  *  |   5   |   16  |   40  |   0  |  N/A |
			| RC4_128      | Stream |     |   16  |   16  |  128  |   0  |  N/A |
			| DES40_CBC    |  Block |  *  |   5   |   8   |   40  |   8  |   8  |
			| DES_CBC      |  Block |     |   8   |   8   |   56  |   8  |   8  |
			| 3DES_EDE_CBC |  Block |     |   24  |   24  |  168  |   8  |   8  |
			+--------------+--------+-----+-------+-------+-------+------+------+
		*/
		switch(cipher)
		{
			case SSL_RC4_128_MD5:
				write_key_size = 16;
				hash_size = 16;
				break;
			case SSL_RC4_128_SHA1:
				write_key_size = 16;
				hash_size = 20;
				break;
			case SSL_AES_128_SHA1:
				write_key_size = 16;
				hash_size = 20;
				iv_size = 16;
				block_size = 16;
				break;
			case SSL_AES_256_SHA1:
				write_key_size = 32;
				hash_size = 20;
				iv_size = 16;
				block_size = 16;
				break;
			default:
				DEBUG_SSL_MSG("Unsupported cipher selected: %d\n", cipher);
				state = ssl_error;
				return -1;
		}

		DEBUG_SSL_MSG("cipher selected: %d\n", cipher);
		pos+=2;
		hello_size -=2;

		if (hello_size == 0)
		{
			DEBUG_SSL_MSG("No place for copression flag!\n");
			state = ssl_error;
			return 0;
		}
		size_t compression_length = pkt[pos++];
		hello_size--;
		if (compression_length > hello_size)
		{
			DEBUG_SSL_MSG("Compression flag is broken!\n");
			state = ssl_error;
			return 0;
		}
		if (compression_length > 0)
		{
			DEBUG_SSL_MSG("compression: %s\n", pkt+pos);
			pos += compression_length;
			hello_size -= compression_length;
			compression_protocol = 1;
			memset(&client_strm, 0, sizeof(client_strm));
			memset(&server_strm, 0, sizeof(server_strm));	
			inflateInit_(&client_strm, ZLIB_VERSION, sizeof(client_strm));
			inflateInit_(&server_strm, ZLIB_VERSION, sizeof(server_strm));
		}
		
		if (server_session_len == 32 && client_session_len == server_session_len && 
			memcmp(client_session, server_session, 32) == 0)
		{
			DEBUG_SSL_MSG("Server has the same session!\n");
			if (findSession(client_session, master_key) )
			{
				DEBUG_SSL_MSG("SESSION FOUND !!!\n");
				memset(pre_master_key, 0, sizeof(pre_master_key));
				state = ssl_server_change_cipher;
				client_session_reuse_state = ssl_client_change_cipher;
				server_session_reuse_state = ssl_server_change_cipher;
				session_reuse = 1;
			} else if (findSessionTicket(client_ticket, master_key, compression_protocol))
			{
				DEBUG_SSL_MSG("SESSION TICKET FOUND !!!\n");
				memset(pre_master_key, 0, sizeof(pre_master_key));
				state = ssl_server_change_cipher;
				client_session_reuse_state = ssl_client_change_cipher;
				server_session_reuse_state = ssl_server_change_cipher;
				session_reuse = 1;
			} else {
				// we do not have an old session. we can nore recover this connection
				DEBUG_SSL_MSG("SESSION CAN NOT BE RECOVERED. Skipping.\n");
				state = ssl_error;
				return 0;
			}
		} else {
			DEBUG_SSL_MSG("Session info not found.\n");
			state = ssl_server_certificate;
			if (pkt_size > original_hello_size + 15)
			{
				state = ssl_client_key_exchange;
			}
		}

		if (hello_size < 2)
			return 1; // we are ok if we got here
		size_t all_ext_length = pkt[pos] << 8 | pkt[pos+1];
		size_t ext_type = 0;
		size_t ext_length = 0;
		hello_size -=2;
		pos+=2;

		if (all_ext_length > hello_size)
		{
			DEBUG_SSL_MSG("ssl broken, no place for extensions! " SIZET_FMT " - " SIZET_FMT " - " SIZET_FMT "\n",
                               all_ext_length, ext_length, hello_size);
			return 0;
		}
		//DEBUG_SSL_MSG("ssl server extensions\n");

		int try_count = 10;
		while (hello_size >= 4 && try_count)
		{
			try_count--;
			ext_type = pkt[pos] << 8 | pkt[pos+1];
			pos+= 2;
			ext_length = pkt[pos] << 8 | pkt[pos+1];
			pos+= 2;
			hello_size-= 4;

			if (ext_length > hello_size)
				return 1;
			if (ext_type == 0x23)
			{
				if (ext_length > 0)
					DEBUG_SSL_MSG("server session ticket: " SIZET_FMT "\n", ext_length);
				else
					DEBUG_SSL_MSG("server session ticket supported\n");
			}
			hello_size -= ext_length;
			pos+=ext_length;
		}

	}
	return pkt_size+5;
}

static int hmac_init = 0;
static unsigned char trans_5C[255];
static unsigned char trans_36[256];

/**
 * Prepare some decoding tables. Addopted from here: http://en.wikipedia.org/wiki/Hash-based_message_authentication_code
 *
 */
void SSLDecoder::HMAC_MD5_Prepare(const unsigned char * key, unsigned int key_len, unsigned char * o_key, unsigned char * i_key )
{
	//unsigned char md5_signature[16];
	size_t i;
	if (hmac_init == 0)
	{
		// init translation table
		for (i = 0; i < 256; i++)
		{
			trans_5C[i] = i ^ 0x5c;
			trans_36[i] = i ^ 0x36;
		}
		hmac_init = 1;
	}
	if (key_len > md5_block_size)
	{
		MD5(key, key_len, o_key);
		key_len = 16;
	}
	memcpy(o_key, key, key_len);
	// append null
	for (i = key_len; i < md5_block_size; i++)
	{
		o_key[i] = '\0';
	}
	for (i = 0; i < md5_block_size; i++)
	{
		i_key[i] = trans_36[o_key[i]];
		o_key[i] = trans_5C[o_key[i]];
	}
}


/**
 * Prepare some decoding tables
 *
 */
void SSLDecoder::HMAC_SHA_Prepare(const unsigned char * key, unsigned int key_len, unsigned char * o_key, unsigned char * i_key )
{
	//unsigned char md5_signature[16];
	size_t i;
	if (hmac_init == 0)
	{
		// init translation table
		for (i = 0; i < 256; i++)
		{
			trans_5C[i] = i ^ 0x5c;
			trans_36[i] = i ^ 0x36;
		}
		hmac_init = 1;
	}
	if (key_len > md5_block_size)
	{
		SHA1(key, key_len, o_key);
		key_len = 16;
	}
	memcpy(o_key, key, key_len);
	// append null
	for (i = key_len; i < (size_t)md5_block_size; i++)
	{
		o_key[i] = '\0';
	}
	for (i = 0; i < (size_t)md5_block_size; i++)
	{
		i_key[i] = trans_36[o_key[i]];
		o_key[i] = trans_5C[o_key[i]];
	}
}


/**
 * Pseudorandom function PRF http://en.wikipedia.org/wiki/Pseudorandom_function.
 * This function is used to generate master key, MAC, IVs and other keys from pre master keys.
 *
 */
void SSLDecoder::PRF(const unsigned char * secret, unsigned int key_len, 
	const unsigned char * seed, const int seed_length,
	unsigned char * result, const size_t result_size)
{
	std::string sha_parameters;
	std::string md5_parameters;
	unsigned char sha_signature[20];
	unsigned char sha_a_signature[20];
	unsigned char md5_signature[16];
	unsigned char md5_a_signature[16];
	unsigned char md5_res[400];
	unsigned char sha_res[400];
	unsigned int s_length = key_len / 2;
	size_t i;
	size_t j;

	//HMAC_CTX ctx;
	unsigned char o_key[md5_block_size];
	unsigned char i_key[md5_block_size];
	/*
			S1 and S2 are the two halves of the secret and each is the same
	   length. S1 is taken from the first half of the secret, S2 from the
	   second half. Their length is created by rounding up the length of the
	   overall secret divided by two; thus, if the original secret is an odd
	   number of bytes long, the last byte of S1 will be the same as the
	   first byte of S2.

		   L_S = length in bytes of secret;
		   L_S1 = L_S2 = ceil(L_S / 2);
	*/
	if (s_length * 2 != key_len)
		s_length += 1;

	HMAC_MD5_Prepare(secret, s_length, o_key, i_key);
/*
	std::string s = "The quick brown fox jumps over the lazy dog";
	HMAC_MD5_Prepare((unsigned char*)"key", 3, md5_o_key, md5_i_key);
	md5_parameters.assign((char *)md5_i_key, sizeof(md5_i_key));
	md5_parameters.append((char*)s.c_str(), s.size());
	MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_signature);
	md5_parameters.assign((char *)md5_o_key, sizeof(md5_o_key));
	md5_parameters.append((char*)md5_signature, sizeof(md5_signature));
	MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_signature);
	*/
	/*
		   First, we define a data expansion function, P_hash(secret, data)
	   which uses a single hash function to expand a secret and seed into an
	   arbitrary quantity of output:

		   P_hash(secret, seed) = HMAC_hash(secret, A(1) + seed) +
								  HMAC_hash(secret, A(2) + seed) +
								  HMAC_hash(secret, A(3) + seed) + ...

	   Where + indicates concatenation.

	   A() is defined as:
		   A(0) = seed
		   A(i) = HMAC_hash(secret, A(i-1))
	*/

	j = (result_size+sizeof(md5_signature))/(sizeof(md5_signature));
	for (i = 0; i < j; i++)
	{
		if (i == 0)
		{
			// generate A(1)
			// A(1) = HMAC_hash(secret, seed)
			md5_parameters.assign((char *)i_key, sizeof(i_key));
			md5_parameters.append((char*)seed, seed_length);
			MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_a_signature);
			md5_parameters.assign((char *)o_key, sizeof(o_key));
			md5_parameters.append((char*)md5_a_signature, sizeof(md5_a_signature));
			MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_a_signature);

			/*
			HMAC_CTX_init(&ctx);
			HMAC_Init_ex(&ctx,(char *)secret, s_length,EVP_md5(), NULL);
			HMAC_Update(&ctx,seed,seed_length);
			HMAC_Final(&ctx,A1,&A1_len);
			*/
		} else {
			// generate A(n)
			// A(i) = HMAC_hash(secret, A(i-1))
			md5_parameters.assign((char *)i_key, sizeof(i_key));
			md5_parameters.append((char*)md5_a_signature, sizeof(md5_a_signature));
			MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_a_signature);
			md5_parameters.assign((char *)o_key, sizeof(o_key));
			md5_parameters.append((char*)md5_a_signature, sizeof(md5_a_signature));
			MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_a_signature);
		}

		md5_parameters.assign((char *)i_key, sizeof(i_key));
		md5_parameters.append((char*)md5_a_signature, sizeof(md5_a_signature));
		md5_parameters.append((char*)seed, seed_length);
		MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_signature);

		md5_parameters.assign((char *)o_key, sizeof(o_key));
		md5_parameters.append((char*)md5_signature, sizeof(md5_signature));
		MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_res+i*sizeof(md5_signature));
	}

	j = (result_size+sizeof(sha_signature))/(sizeof(sha_signature));
	const unsigned char * secret2 = secret + key_len - s_length;
	HMAC_SHA_Prepare(secret2, s_length, o_key, i_key);
	for (i = 0; i < j; i++)
	{
		if (i == 0)
		{
			// generate A(1)
			// A(1) = HMAC_hash(secret, seed)
			sha_parameters.assign((char *)i_key, sizeof(i_key));
			sha_parameters.append((char*)seed, seed_length);
			SHA1( (unsigned char*)sha_parameters.c_str(), sha_parameters.size(), sha_a_signature);
			sha_parameters.assign((char *)o_key, sizeof(o_key));
			sha_parameters.append((char*)sha_a_signature, sizeof(sha_a_signature));
			SHA1( (unsigned char*)sha_parameters.c_str(), sha_parameters.size(), sha_a_signature);
		} else {
			// generate A(n)
			// A(i) = HMAC_hash(secret, A(i-1))
			sha_parameters.assign((char *)i_key, sizeof(i_key));
			sha_parameters.append((char*)sha_a_signature, sizeof(sha_a_signature));
			SHA1( (unsigned char*)sha_parameters.c_str(), sha_parameters.size(), sha_a_signature);
			sha_parameters.assign((char *)o_key, sizeof(o_key));
			sha_parameters.append((char*)sha_a_signature, sizeof(sha_a_signature));
			SHA1( (unsigned char*)sha_parameters.c_str(), sha_parameters.size(), sha_a_signature);
		}

		sha_parameters.assign((char *)i_key, sizeof(i_key));
		sha_parameters.append((char*)sha_a_signature, sizeof(sha_a_signature));
		sha_parameters.append((char*)seed, seed_length);
		SHA1( (unsigned char*)sha_parameters.c_str(), sha_parameters.size(), sha_signature);

		sha_parameters.assign((char *)o_key, sizeof(o_key));
		sha_parameters.append((char*)sha_signature, sizeof(sha_signature));
		SHA1( (unsigned char*)sha_parameters.c_str(), sha_parameters.size(), sha_res+i*sizeof(sha_signature));
	}

	// xor everything together
	for (i = 0; i <result_size; i++)
	{
		result[i] = md5_res[i] ^ sha_res[i];
	}
}

/**
 * TLS1GenerateMasterkey() function is used to generate master key for TLS1.0 and TLS1.1 out of pre master key received and client and server random bytes
 *
 */
ssl_state SSLDecoder::TLS1GenerateMasterkey(const unsigned char * pre_master_key, const int key_len)
{
	/*

       PRF(secret, label, seed) = P_MD5(S1, label + seed) XOR
                                  P_SHA-1(S2, label + seed);

       master_secret = PRF(pre_master_secret, "master secret",
                           ClientHello.random + ServerHello.random)
       [0..47];
	*/
	std::string seed = "master secret";

	seed.append( (char*)client_random, sizeof(client_random));
	seed.append( (char*)server_random, sizeof(server_random));
	PRF(pre_master_key, (unsigned int) key_len, (unsigned char*) seed.c_str(), seed.size(), (unsigned char*) master_key, sizeof(master_key));

	return ssl_client_change_cipher;
}

/**
 * TLS1GenerateKeys() function is used to generate MAC, keys and IV out of master key used for TLS1.0 and TLS1.1.
 *
 */
bool SSLDecoder::TLS1GenerateKeys()
{
	/*
	   To generate the key material, compute

		   key_block = PRF(SecurityParameters.master_secret,
							  "key expansion",
							  SecurityParameters.server_random +
							  SecurityParameters.client_random);

	   until enough output has been generated. Then the key_block is
	   partitioned as follows:

		   client_write_MAC_secret[SecurityParameters.hash_size]
		   server_write_MAC_secret[SecurityParameters.hash_size]
		   client_write_key[SecurityParameters.key_material_length]
		   server_write_key[SecurityParameters.key_material_length]
		   client_write_IV[SecurityParameters.IV_size]
		   server_write_IV[SecurityParameters.IV_size]

	   The client_write_IV and server_write_IV are only generated for non-
	   export block ciphers. For exportable block ciphers, the
	   initialization vectors are generated later, as described below. Any
	   extra key_block material is discarded.
	*/
	std::string seed = "key expansion";
	unsigned char keys[200] = {0};
	seed.append( (char*)server_random, sizeof(server_random));
	seed.append( (char*)client_random, sizeof(client_random));

	PRF((unsigned char*)master_key,sizeof(master_key),(unsigned char*)seed.c_str(), seed.size(), (unsigned char *) keys, sizeof(keys));

	int size = 0;
	if (hash_size > 0)
	{
		memcpy(client_write_mac_secret, keys+size, hash_size);
		size+= hash_size;
		memcpy(server_write_mac_secret, keys+size, hash_size);
		size+= hash_size;
	}
	memcpy(client_write, keys+size, write_key_size);
	size+= write_key_size;
	memcpy(server_write, keys+size, write_key_size);
	size+= write_key_size;
	if (iv_size > 0)
	{
		memcpy(client_write_iv, keys+size, iv_size);
		size+= iv_size;
		memcpy(server_write_iv, keys+size, iv_size);
		size+= iv_size;
	}

	/*
	   Exportable encryption algorithms (for which CipherSpec.is_exportable
	   is true) require additional processing as follows to derive their
	   final write keys:

		   final_client_write_key =
		   PRF(SecurityParameters.client_write_key,
									  "client write key",
									  SecurityParameters.client_random +
									  SecurityParameters.server_random);
		   final_server_write_key =
		   PRF(SecurityParameters.server_write_key,
									  "server write key",
									  SecurityParameters.client_random +
									  SecurityParameters.server_random);
	*/
	return true;
}

/**
 * SSL3GenerateMasterkey() function is used to generate master key used in SSLv3.
 *
 */
ssl_state SSLDecoder::SSL3GenerateMasterkey(const unsigned char * pre_master_key, const int key_len)
{
	/*
		master_secret =
		MD5(pre_master_secret + SHA('A' + pre_master_secret +
			ClientHello.random + ServerHello.random)) +
		MD5(pre_master_secret + SHA('BB' + pre_master_secret +
			ClientHello.random + ServerHello.random)) +
		MD5(pre_master_secret + SHA('CCC' + pre_master_secret +
			ClientHello.random + ServerHello.random));
	*/

	std::string sha_parameters;
	std::string md5_parameters;
	unsigned char sha_signature[20];
	unsigned char md5_signature[16];
	sha_parameters.append( (char*)pre_master_key, key_len);
	sha_parameters.append( (char*)client_random, sizeof(client_random));
	sha_parameters.append( (char*)server_random, sizeof(server_random));

	std::string sha_str = "A";
	sha_str.append(sha_parameters);
	SHA1((unsigned char*) sha_str.c_str(), sha_str.size(), sha_signature);

	md5_parameters.append( (char*) pre_master_key, key_len);
	md5_parameters.append( (char*) sha_signature, sizeof(sha_signature));
	MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_signature);
	memcpy( master_key, md5_signature, sizeof(md5_signature));

	sha_str = "BB";
	sha_str.append(sha_parameters);
	SHA1( (unsigned char*)sha_str.c_str(), sha_str.size(), sha_signature);

	md5_parameters.clear();
	md5_parameters.append( (char*)pre_master_key, key_len);
	md5_parameters.append( (char*)sha_signature, sizeof(sha_signature));
	MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_signature);
	memcpy( master_key+16, md5_signature, sizeof(md5_signature));
			
	sha_str = "CCC";
	sha_str.append(sha_parameters);
	SHA1( (unsigned char*)sha_str.c_str(), sha_str.size(), sha_signature);

	md5_parameters.clear();
	md5_parameters.append( (char*)pre_master_key, key_len);
	md5_parameters.append( (char*)sha_signature, sizeof(sha_signature));
	MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_signature);
	//master_key.append( (char*)md5_signature, sizeof(md5_signature));
	memcpy( master_key+32, md5_signature, sizeof(md5_signature));
	return ssl_client_change_cipher;
}

/**
 * This function is actually not used.
 *
 */
bool SSLDecoder::SSL3ValidateFinished(bool from_client, const unsigned char * keys, int key_len)
{
	/*
	md5_hash:  MD5(master_secret + pad2 + MD5(handshake_messages + Sender
		+ master_secret + pad1));

	sha_hash:  SHA(master_secret + pad2 + SHA(handshake_messages + Sender
		+ master_secret + pad1));

	enum { client(0x434C4E54), server(0x53525652) } Sender;
	pad_1:  The character 0x36 repeated 48 times for MD5 or 40 times for SHA.
	pad_2:  The character 0x5c repeated 48 times for MD5 or 40 times for SHA.
	*/
	static unsigned char client_sender[] = "\x43\x4c\x4e\x54";
	static unsigned char server_sender[] = "\x53\x52\x56\x52";
	static unsigned char pad1[] = "\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
								"\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
								"\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36";
	static unsigned char pad2[] = "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
							"\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
							"\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c";
	unsigned char md5_sig[16];
	unsigned char sha1_sig[20];
	std::string md5_parameters = handshake_msg;

	if (from_client)
	{
		md5_parameters.append( (char*) client_sender, sizeof(client_sender));
	} else {
		md5_parameters.append( (char*) server_sender, sizeof(server_sender));
	}
	md5_parameters.append( (char*) master_key, sizeof(master_key));
	md5_parameters.append( (char*) pad1, 48);
	MD5( (unsigned char *)md5_parameters.c_str(), md5_parameters.size(), md5_sig);
	SHA1( (unsigned char *)md5_parameters.c_str(), md5_parameters.size()-8, sha1_sig), 

	md5_parameters.clear();
	md5_parameters.append( (char*) master_key, sizeof(master_key) );
	md5_parameters.append( (char*) pad2, 48);
	md5_parameters.append( (char*) md5_sig, sizeof(md5_sig));
	MD5( (unsigned char *)md5_parameters.c_str(), md5_parameters.size(), md5_sig);
	DEBUG_SSL_MSG("expected md5:\n");
	DEBUG_SSL_HEX(md5_sig, sizeof(md5_sig));

	std::string sha1_parameters;
	sha1_parameters.append( (char*) master_key, sizeof(master_key) );
	sha1_parameters.append( (char*) pad2, 40);
	sha1_parameters.append( (char*)sha1_sig, sizeof(sha1_sig));
	SHA1( (unsigned char*)sha1_parameters.c_str(), sha1_parameters.size(), sha1_sig);
	DEBUG_SSL_MSG("expected sha1:\n");
	DEBUG_SSL_HEX(sha1_sig, sizeof(sha1_sig));
	return true;
}

/**
 * This function is used to generate MAC, keys and IV out of master key used for SSLv3.
 *
 */
bool SSLDecoder::SSL3GenerateKeys()
{
	/*
        key_block =
          MD5(master_secret + SHA(`A' + master_secret +
                                  ServerHello.random +
                                  ClientHello.random)) +
          MD5(master_secret + SHA(`BB' + master_secret +
                                  ServerHello.random +
                                  ClientHello.random)) +
          MD5(master_secret + SHA(`CCC' + master_secret +
                                  ServerHello.random +
                                  ClientHello.random)) + [...];
	*/

	std::string keys;
	std::string sha_str;
	std::string sha_parameters;
	std::string md5_parameters;
	unsigned char sha_signature[20];
	unsigned char md5_signature[16];

	sha_parameters.append( (char*)master_key, sizeof(master_key));
	sha_parameters.append( (char*)server_random, sizeof(server_random));
	sha_parameters.append( (char*)client_random, sizeof(client_random));

	sha_str = "A";
	sha_str.append(sha_parameters);
	SHA1((unsigned char*) sha_str.c_str(), sha_str.size(), sha_signature);

	md5_parameters.clear();
	md5_parameters.append( (char*) master_key, sizeof(master_key));
	md5_parameters.append( (char*) sha_signature, sizeof(sha_signature));
	MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_signature);
	keys.append( (char*) md5_signature, sizeof(md5_signature));

	sha_str = "BB";
	sha_str.append(sha_parameters);
	SHA1((unsigned char*) sha_str.c_str(), sha_str.size(), sha_signature);

	md5_parameters.clear();
	md5_parameters.append( (char*) master_key, sizeof(master_key));
	md5_parameters.append( (char*) sha_signature, sizeof(sha_signature));
	MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_signature);
	keys.append( (char*) md5_signature, sizeof(md5_signature));

	sha_str = "CCC";
	sha_str.append(sha_parameters);
	SHA1((unsigned char*) sha_str.c_str(), sha_str.size(), sha_signature);

	md5_parameters.clear();
	md5_parameters.append( (char*) master_key, sizeof(master_key));
	md5_parameters.append( (char*) sha_signature, sizeof(sha_signature));
	MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_signature);
	keys.append( (char*) md5_signature, sizeof(md5_signature));

	sha_str = "DDDD";
	sha_str.append(sha_parameters);
	SHA1((unsigned char*) sha_str.c_str(), sha_str.size(), sha_signature);

	md5_parameters.clear();
	md5_parameters.append( (char*) master_key, sizeof(master_key));
	md5_parameters.append( (char*) sha_signature, sizeof(sha_signature));
	MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_signature);
	keys.append( (char*) md5_signature, sizeof(md5_signature));

	sha_str = "EEEEE";
	sha_str.append(sha_parameters);
	SHA1((unsigned char*) sha_str.c_str(), sha_str.size(), sha_signature);

	md5_parameters.clear();
	md5_parameters.append( (char*) master_key, sizeof(master_key));
	md5_parameters.append( (char*) sha_signature, sizeof(sha_signature));
	MD5( (unsigned char*)md5_parameters.c_str(), md5_parameters.size(), md5_signature);
	keys.append( (char*) md5_signature, sizeof(md5_signature));

		/*
			client_write_MAC_secret[CipherSpec.hash_size]
			server_write_MAC_secret[CipherSpec.hash_size]
			client_write_key[CipherSpec.key_material]
			server_write_key[CipherSpec.key_material]
			client_write_IV[CipherSpec.IV_size] / * non-export ciphers * /
			server_write_IV[CipherSpec.IV_size] / * non-export ciphers * /

			Any extra key_block material is discarded.

			Exportable encryption algorithms (for which CipherSpec.is_exportable
			is true) require additional processing as follows to derive their
			final write keys:

				final_client_write_key = MD5(client_write_key +
												ClientHello.random +
												ServerHello.random);
				final_server_write_key = MD5(server_write_key +
												ServerHello.random +
												ClientHello.random);
		*/
	int size = 0;
	if (hash_size > 0)
	{
		memcpy(client_write_mac_secret, keys.c_str()+size, hash_size);
		size+= hash_size;
		memcpy(server_write_mac_secret, keys.c_str()+size, hash_size);
		size+= hash_size;
	}
	memcpy(client_write, keys.c_str()+size, write_key_size);
	size+= write_key_size;
	memcpy(server_write, keys.c_str()+size, write_key_size);
	size+= write_key_size;

        if (iv_size > 0)
        {
                if (iv_size * 2 > keys.size()-size)
                {
                        DEBUG_SSL_MSG("not enough data for keys !!!!!!!!!!!!!!!!!\n");
                }
		memcpy(client_write_iv, keys.c_str()+size, iv_size);
		size+= iv_size;
		memcpy(server_write_iv, keys.c_str()+size, iv_size);
		size+= iv_size;
        }

	return true;
}

#define CHUNK 4048*4
#define windowBits 15
#define GZIP_ENCODING 16

/**
 * Decompress compressed packet. Using zlib for that.
 *
 */
bool SSLDecoder::ZLIBDecompress(z_stream & strm, const unsigned char * from, unsigned int size,std::string & result)
{
	unsigned char out[CHUNK];
	int status=-1;
	int have;

	strm.avail_in = size;
	//strm.total_in = size;
	strm.next_in = (Bytef*)from;

	/*
	strm.avail_out = sizeof(out);
	strm.next_out = out;
	status = inflate(&strm, Z_SYNC_FLUSH);
	//inflateEnd (& strm);
	if (status == 0)
	{
		result.append((char*)out, strm.total_out);
		return true;
	}
	*/

	do {
        	strm.avail_out = CHUNK;
        	strm.next_out = out;
                status = inflate( &strm, Z_SYNC_FLUSH ); // Z_FINISH
                if (status < 0)
                        return false;
        	have = CHUNK - strm.avail_out;
                result.append((char*)out, have);
    	}
    	while (strm.avail_out == 0);
	// inflateEnd (& strm);

	return true;
}

/*
	When a connection is established by resuming a session, new
	   ClientHello.random and ServerHello.random values are hashed with the
	   session's master_secret. Provided that the master_secret has not been
	   compromised and that the secure hash operations used to produce the
	   encryption keys and MAC secrets are secure, the connection should be
	   secure and effectively independent from previous connections.
	   Attackers cannot use known encryption keys or MAC secrets to
	   compromise the master_secret without breaking the secure hash
	   operations (which use both SHA and MD5).
*/
