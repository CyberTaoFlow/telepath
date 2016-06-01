
#ifdef WIN32
#include <openssl/applink.c> // To prevent crashing (see the OpenSSL FAQ)
#endif
#include <openssl/bio.h> // BIO objects for I/O
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/pkcs12.h>

#include "ssl_decoder.h"
#include "utils.h"

#define KEYFILE "private.txt"
#define CERTFILE "cert.txt"

#ifndef WIN32
#include <arpa/inet.h>
#endif

//static const unsigned int port_443 = ntohs(443);
//RSA * get_ssl_cert(const char * server_ip, unsigned int port);

RSA * initRSA();
extern unsigned char * base64_decode (const char * str, size_t in_len, size_t & res);
/*
RSA * get_ssl_cert(const char * server_ip, unsigned int port)
{
	static RSA * rsa = initRSA();

	if (port == port_443)
	{
		return rsa;
	}
	return NULL;
}
*/

/*
SSLDecoder * get_ssl_decoder(const char * server_ip, unsigned int port)
{
	static RSA * rsa = initRSA();
	SSLDecoder * decoder = NULL;

	if (port == port_443)
	{
		SSLDecoder * decoder = new SSLDecoder(rsa);
	}
	return decoder;
}
*/

/**
 * password_cb() is a callback function used to read pasword when opening private key.
 */
static int password_cb(char *buf,int num, int rwflag,void *userdata)
{
	if (userdata == 0 || *((char*)userdata) == '\0')
	{
		strcpy(buf, "");
		return 0;
	}
	memcpy(buf, userdata, strlen( (char*) userdata));
        return (strlen(buf));
}


/**
 * loadRSA() function is used from the bro to load private certificates. It can load PFX ceritificates too.
 *
 * @param priv_key points to the base64 encoded string with private key
 * @param pwd is a password to use when loading private key. It can be an empty string.
 * @return loaded and parsed private key
 */
RSA * loadRSA(std::string & priv_key, std::string & pwd)
{
        EVP_PKEY *pkey = NULL;
        RSA * rsa = NULL;
        BIO * key=NULL;
	size_t size = 0;
	static int loaded = 0;

	//initRSA();

	if (loaded == 0)
	{
		ERR_load_crypto_strings();
		OpenSSL_add_all_algorithms();
		loaded = 1;
	}
	unsigned char * b = base64_decode(priv_key.c_str(), strlen(priv_key.c_str()), size);
	if (!b)
	{
		DEBUG_SSL_MSG("Failed to perform base64 decode operation\n");
		return NULL;
	}
	key = BIO_new_mem_buf( (void *) b, size );
	if (b[0] == '-')
	{
		DEBUG_SSL_MSG("Pri key:\n%s\n", b);
	}
	//delete [] b;
	pkey=PEM_read_bio_PrivateKey(key,NULL, (pem_password_cb *)password_cb, (void *) pwd.c_str());
        if (pkey == NULL)
        {
		// PEM_read_bio_PrivateKey Destoryis our BIO object !
		BIO_free(key);
		key = BIO_new_mem_buf( (void *) b, size );
		// Failed to load
		// try to load pfx file (pkcs12)
		//X509 *cert;
		PKCS12 *p12;
		//STACK_OF(X509) *ca = NULL;
		p12 = d2i_PKCS12_bio(key, NULL);
		if (p12 == NULL)
		{	
			char buffer[250];
                	ERR_error_string(ERR_get_error(), buffer);
			DEBUG_SSL_MSG("error: %s\n", buffer);
                	DEBUG_SSL_MSG("Failed to load private key\n");
			free(b);
			BIO_free(key);
			return NULL;
		}
		PKCS12_parse(p12, pwd.c_str(), &pkey, NULL, NULL);
		PKCS12_free(p12);
		if (pkey == NULL)
		{
			char buffer[250];
			ERR_error_string(ERR_get_error(), buffer);
			DEBUG_SSL_MSG("error: %s\n", buffer);
			DEBUG_SSL_MSG("Failed to load private key\n");
			free(b);
			BIO_free(key);
			return NULL;
		}
        }
	free(b);
	rsa = EVP_PKEY_get1_RSA(pkey);
        EVP_PKEY_free(pkey);
	BIO_free(key);
	if (rsa)
		DEBUG_SSL_MSG("SSL certificate loaded !\n");
	return rsa;
}

/**
 * initRSA() function is used to init internal openssl structures. Currently it is not used.
 */
RSA * initRSA()
{
	EVP_PKEY *pkey = NULL;
	RSA * rsa = NULL;
	BIO *key=NULL;
	const char * file = KEYFILE;
	//PW_CB_DATA cb_data;

	key=BIO_new(BIO_s_file());
	if (key == NULL)
	{
		DEBUG_SSL_MSG("failed to create new BIO object !\n");
		return NULL;
	}
	if (BIO_read_filename(key,file) <= 0)
	{
		DEBUG_SSL_MSG("failed to read certificate file: %s\n", file);
		//BIO_DEBUG_SSL_MSG(bio_err, "Error opening %s %s\n", "private key", file);
		//ERR_print_errors(bio_err);
		return NULL;
	}

	//if (format == FORMAT_PEM)
	//{
	pkey=PEM_read_bio_PrivateKey(key,NULL, (pem_password_cb *)password_cb, NULL); //&cb_data);
	//}
	/*
	else if (format == FORMAT_PKCS12)
	{
	if (!load_pkcs12(err, key, key_descrip, (pem_password_cb *)password_callback, &cb_data, &pkey, NULL, NULL))
	{
		// error
	}
	}
	*/
	if (pkey == NULL)
	{
		DEBUG_SSL_MSG("failed to load private key %s\n", file);
		return NULL;
	}
	rsa = EVP_PKEY_get1_RSA(pkey);
	EVP_PKEY_free(pkey);
	return rsa;
}
