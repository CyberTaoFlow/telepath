// ssl_server.cpp : Defines the entry point for the console application.
//

// list of ciphers http://www.openssl.org/docs/apps/ciphers.html


//#include "stdafx.h"

// This is test project main file.

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <stdio.h>

#include <openssl/applink.c> // To prevent crashing (see the OpenSSL FAQ)
#include <openssl/bio.h> // BIO objects for I/O
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include "ssl_decoder.h"
#include "utils.h"

#define SERVER_PORT 1235
#define PORT 1234
#define BUFFSIZE 10240
#define PASSWORD ""
#define KEYFILE "private.txt"
#define CERTFILE "cert.txt"

#define KEYFILE "C:\\Users\\yuli\\Desktop\\code\\SSL\\ssl_keys\\private.txt"
#define CERTFILE "C:\\Users\\yuli\\Desktop\\code\\SSL\\ssl_keys\\cert.txt"

BIO *bio_err=0;
static char *pass;

static int password_cb(char *buf,int num,
	int rwflag,void *userdata);

/*The password code is not thread safe*/
static int password_cb(char *buf,int num,
	int rwflag,void *userdata)
{
	char * pass = "";
	if(num<strlen(pass)+1)
		return(0);

	strcpy(buf,pass);
	return(strlen(pass));
}

/* A simple error and exit routine*/
int err_exit(char *str)
{
	fprintf(stderr,"%s\n",str);
	exit(0);
}

/* Print SSL errors and exit*/
int berr_exit(char *str)
{
	//ERR_print_errors_fp(stderr);
	//BIO_DEBUG_SSL_MSG(bio_err,"%s\n",str);
	//ERR_print_errors(bio_err);
	exit(0);
}



int tcp_connect(char *host, int port)
{
	struct hostent *hp;
	struct sockaddr_in addr;
	int sock;

	if(!(hp=gethostbyname(host)))
		berr_exit("Couldn't resolve host");
	memset(&addr,0,sizeof(addr));
	addr.sin_addr=*(struct in_addr*)
		hp->h_addr_list[0];
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);

	if((sock=socket(AF_INET,SOCK_STREAM,
		IPPROTO_TCP))<0)
		err_exit("Couldn't create socket");
	if(connect(sock,(struct sockaddr *)&addr,
		sizeof(addr))<0)
		err_exit("Couldn't connect socket");

	return sock;
}


int tcp_listen()
{
	int sock;
	struct sockaddr_in sin;
	int val=1;

	if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
		err_exit("Couldn't make socket");

	memset(&sin,0,sizeof(sin));
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(PORT);
	//setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,
	//  &val,sizeof(val));

	if(bind(sock,(struct sockaddr *)&sin,
		sizeof(sin))<0)
		berr_exit("Couldn't bind");
	listen(sock,5);  

	return(sock);
}

typedef struct pw_cb_data
{
	const void *password;
	const char *prompt_info;
} PW_CB_DATA;

RSA * initRSA()
{
	EVP_PKEY *pkey = NULL;
	RSA * rsa = NULL;
	BIO *key=NULL;
	char * file = KEYFILE;
	PW_CB_DATA cb_data;

	key=BIO_new(BIO_s_file());
	if (key == NULL)
	{
		exit(0);
	}
	if (BIO_read_filename(key,file) <= 0)
	{
		DEBUG_SSL_MSG("Error opening key file: %s\n", file);
		return NULL;
	}

	//if (format == FORMAT_PEM)
	//{
	pkey=PEM_read_bio_PrivateKey(key,NULL, (pem_password_cb *)password_cb, &cb_data);
	//}
	/*
	else if (format == FORMAT_PKCS12)
	{
	if (!load_pkcs12(err, key, key_descrip, (pem_password_cb *)password_callback, &cb_data, &pkey, NULL, NULL))
	{
	exit();
	}
	}
	*/
	rsa = EVP_PKEY_get1_RSA(pkey);
	EVP_PKEY_free(pkey);
	return rsa;
}

int main(int argc, char* argv[])
{
	WSADATA wsaData = {0};
	int iResult;
	int sock;
	int frontend;
	int backend;
	int max_sock;
	BIO *sbio;
	SSL_CTX *ctx;
	SSL *ssl;
	int r;
	int n;
	unsigned char buff[BUFFSIZE];
	//pid_t pid;
	RSA * rsa = NULL;

#ifdef WIN32
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		DEBUG_SSL_MSG("WSAStartup failed: %d\n", iResult);
		return 1;
	}
#endif

	if (bio_err == NULL)
		if ((bio_err=BIO_new(BIO_s_file())) != NULL)
			BIO_set_fp(bio_err,stderr,BIO_NOCLOSE|BIO_FP_TEXT);

	rsa = initRSA();
	if (rsa == NULL)
		exit(-1);

	sock=tcp_listen();
	SSLDecoder * decoder = NULL;

	fd_set read_fds;
	fd_set write_fds;

	while(1)
	{
again:
		if (decoder != NULL)
		{
			delete decoder;
		}
		decoder = new SSLDecoder(rsa);
		DEBUG_SSL_MSG("-------------------------------------\n");
		DEBUG_SSL_MSG("accepting new connection\n");
		if( (frontend=accept(sock,0,0)) <0)
		{
			DEBUG_SSL_MSG("error in accept");
			exit(0);
			//DEBUG_SSL_MSG("ssl error: %d\n",SSL_get_error(ctx, ));
			//err_exit("Problem accepting");
		}
		backend = tcp_connect("127.0.0.1", SERVER_PORT);
		timeval tv = {0};
		max_sock = backend;
		if (backend > frontend)
			max_sock = backend+1;
		else
			max_sock = frontend+1;

		while(1)
		{
			FD_ZERO(&read_fds);
			FD_SET(backend, &read_fds);
			FD_SET(frontend, &read_fds);
			tv.tv_usec = 100;

			select(max_sock, &read_fds, NULL, 0, &tv);

			if (FD_ISSET(frontend,&read_fds))
			{
				n = recv(frontend, (char*) buff, sizeof(buff), 0);
				if (n <= 0)
				{
					DEBUG_SSL_MSG("error in recv from frontend\n");
					closesocket(frontend);
					closesocket(backend);
					goto again;
				}
				while ( (r = decoder->CheckIfReady( (unsigned char *) buff, n)) == -1)
				{
					int n2 = recv(frontend, (char *)buff+n, sizeof(buff)-n, 0);
					if (n2 <= 0)
					{
						DEBUG_SSL_MSG("error in recv from frontend\n");
						closesocket(frontend);
						closesocket(backend);
						goto again;
						exit(0);
					}
					n += n2;
				}
				decoder->ParseClientPackets( (unsigned char *) buff, n);
				//DEBUG_SSL_HEX( (const unsigned char *)buff, n);
				n = send(backend, (char*) buff, n, 0);
				if (n <= 0)
				{
					DEBUG_SSL_MSG("error in send to frontend\n");
					closesocket(frontend);
					closesocket(backend);
					goto again;
				}
			}
			if (FD_ISSET(backend,&read_fds))
			{
				// read response from server
				n = recv(backend, (char*) buff, sizeof(buff), 0);
				if (n <= 0)
				{
					DEBUG_SSL_MSG("error in recv from backend\n");
					closesocket(frontend);
					closesocket(backend);
					goto again;
				}

				while ( (r = decoder->CheckIfReady( (unsigned char *) buff, n)) == -1)
				{
					int n2 = recv(backend, (char *)buff+n, sizeof(buff)-n, 0);
					if (n2 <= 0)
					{
						DEBUG_SSL_MSG("error in recv from backend\n");
						closesocket(frontend);
						closesocket(backend);
						goto again;
						exit(0);
					}
					n += n2;
				}
				decoder->ParseServerPackets( (unsigned char *) buff, n);
				//DEBUG_SSL_HEX( (const unsigned char *)buff, n);
				n = send(frontend, (char*)buff, n, 0);
				if (n <= 0)
				{
					DEBUG_SSL_MSG("error in send to frontend\n");
					closesocket(frontend);
					closesocket(backend);
					goto again;
				} else {
					if (buff[0] == 0x17)
					{
						unsigned int pkt_size = buff[3] << 8 | buff[4];
						if (pkt_size+5 <= n)
						{
							// server response was here,  close socket for now
							closesocket(frontend);
							closesocket(backend);
							goto again;
						}

					}
				}
			}
		}
		//
	}
	//destroy_ctx(ctx);
	exit(0);

	return 0;
}

