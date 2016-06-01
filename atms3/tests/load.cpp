#include <openssl/bio.h> // BIO objects for I/O
#include <openssl/err.h>

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/pkcs12.h>
#include <string.h>

/*The password code is not thread safe*/
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

int main()
{
	FILE *fp;
        EVP_PKEY *pkey = NULL;
        RSA * rsa = NULL;
        BIO * key=NULL;
	PKCS12 *p12 = NULL;
        ssize_t size = 0;
        ERR_load_crypto_strings();
        OpenSSL_add_all_algorithms();
	fp = fopen("./wincert.pfx", "rb");	
	if (fp == NULL)
	{
		printf("Failed to open cert file\n");
		exit(-1);
	}
        fseek (fp, 0 , SEEK_END);
        size = ftell (fp);
        rewind (fp);
        if (size <= 0)
        {
            fclose(fp);
            return 0;
        }
	printf("File size: %d\n", size);
	char * buf[size+1];
	fread(buf,size,1,fp);
        key = BIO_new_mem_buf( (void *) buf, size );

        pkey=PEM_read_bio_PrivateKey(key,NULL, (pem_password_cb *)password_cb, (void *) "test1234");
	if (pkey == NULL)
	{
		printf("Failed to load private key (this is ok!)\n");
		BIO_free(key);
		key = BIO_new_mem_buf( (void *) buf, size );
	}

        p12 = d2i_PKCS12_bio(key, NULL);
        if (p12 == NULL)
        {
            char buffer[250];
                        ERR_error_string(ERR_get_error(), buffer);
                        printf("error: %s\n", buffer);
                        printf("Failed to load private key\n");
                        BIO_free(key);
                        return NULL;
        }
        printf("p12 created !\n");
                PKCS12_parse(p12, "test1234", &pkey, NULL, NULL);
                PKCS12_free(p12);
                if (pkey == NULL)
                {
                        char buffer[250];
                        ERR_error_string(ERR_get_error(), buffer);
                        printf("error: %s\n", buffer);
                        printf("Failed to load private key\n");
                        BIO_free(key);
                        return NULL;
                }
        rsa = EVP_PKEY_get1_RSA(pkey);
        EVP_PKEY_free(pkey);
        BIO_free(key);
        if (rsa)
                printf("SSL certificate loaded !\n");
        printf("loaded !\n");
}

