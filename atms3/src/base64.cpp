/*
 *
 *
 * Encoding/decoding to/from base64
 * taken from google :) no comments
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const static char base64ABC[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
typedef unsigned char uchar_t;
#define S_(a) S[n * 3 + a]
#define S__(a,b) (S[n * 3 + a] & b)
#define S__l(a,b,c) ((S[n * 3 + a] & b) << c)
#define B1 buffer[n * 4] = base64ABC[ (S_(0) >> 2) ];
#define B2 buffer[n * 4 + 1] = base64ABC [ (S__l(0,0x3,4)) | S_(1) >> 4 ];
#define B3 buffer[n * 4 + 2] = base64ABC [ S__l(1,0x0F,2) | S_(2) >> 6];
#define B4 buffer[n * 4 + 3] = base64ABC [ S__(2,0x3F) ];

unsigned char * base64_code (unsigned char * S, int SIZE = 0)
{
    uchar_t c, *buffer;
    int n;
   
    c = (SIZE == 0) ? strlen ((const char *) S) % 3 : SIZE % 3;
    SIZE = (SIZE == 0) ? strlen ((const char *) S) / 3 : SIZE / 3;
   
    buffer = new uchar_t [SIZE * 4 + 5];
   
    for (n = 0; n <    SIZE; n++)
    { B1; B2; B3; B4; }
   
    switch (c)
    {
    case 1:
        B1; B2;   
        buffer[n * 4 + 2] = '=';
        buffer[n * 4 + 3] = '=';
        n++;
        break;
    case 2:
        B1; B2; B3;
        buffer[n * 4 + 3] = '=';
        n++;
        break;
    }
    buffer[n * 4] = 0;
//    printf ("%s---\n", buffer);
    return buffer;
}
#define BASE64(a) ((unsigned char) (strchr (base64ABC, S[n * 4 + a]) - base64ABC))
unsigned char * base64_decode2 (unsigned char * S, size_t SIZE = 0)
{
//	printf("base64_decode():\n%s\n",S);
    	unsigned char * ptr;
    	size_t n;
	size_t i;
   
    	//if (SIZE % 4)
        //	return NULL;
   
	SIZE = SIZE ? SIZE / 4 : strlen ((const char *) S) / 4;
 
    	ptr = new unsigned char [SIZE * 4 + 5];

//Added Yakov: cleaning memory before decoding
	memset(ptr,'\0',SIZE*4+5);
//!Added Yakov: cleaning memory before decoding   

    	for (i=0, n = 0; n < SIZE; n++, i++)
    	{
		if (S[n] == '\r' || S[n] == '\n')
			continue;
        	ptr[i * 3] = (BASE64(0) << 2) | ( BASE64(1) >> 4);
        	if (S[n * 4 +2] != '=')
        	{
            		ptr[i * 3 + 1] = ((BASE64(1) & 0x0F) << 4) | (BASE64(2) >> 2);
            		if (S[n * 4 + 3] != '=')
			{
                		ptr[i * 3 + 2] = ((BASE64(2) & 0x03) << 6) | BASE64 (3);
            		} else {
                		ptr[i * 3 + 2] = 0;
			}
        	} else {
            		ptr[i * 3 +1] = 0;
		}
    	}
   
    	i++;
    	ptr [i * 3] = 0;
//    	printf("base64_decode() end :\n%s\n",ptr);
    	return ptr;
}

//  http://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c

static char encoding_table[256] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

static unsigned char decoding_table[256];

unsigned char *base64_decode(const char *d, size_t input_length, size_t & output_length)
{
    static int build = 0;
    size_t i;
    size_t j;
    unsigned int sextet_a, sextet_b, sextet_c, sextet_d, triple;
    const unsigned char * data = (const unsigned char *) d;

    if (build == 0)
    {
        for (j = 0; j < 256; j++)
            decoding_table[(unsigned char) encoding_table[j]] = j;
        build = 1;
    }

    //if (input_length % 4 != 0) return NULL;

    output_length = (input_length / 4) * 3 + 5;
    //if (data[input_length - 1] == '=') output_length--;
    //if (data[input_length - 2] == '=') output_length--;

    unsigned char *decoded_data = (unsigned char*) malloc(output_length);
    if (decoded_data == NULL)
      return NULL;
    i = 0;
    while (data[i] == '\r' || data[i] == '\n') { i++; }

    //size_t correction = 0;
    for ( j = 0; i < input_length;)
    {
        sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
	while (data[i] == '\r' || data[i] == '\n') { i++; }
	if (i >= input_length)
	{
		sextet_b = 0;
		sextet_c = 0;
		sextet_d = 0;
		//correction = 2;
	} else {
        	sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
		while (data[i] == '\r' || data[i] == '\n') { i++; }
		if (i >= input_length)
		{
			sextet_c = 0;
			sextet_d = 0;
			//correction = 1;
		} else {
        		sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
			while (data[i] == '\r' || data[i] == '\n') { i++; }
			if (i >= input_length)
			{
				sextet_d = 0;
			} else {
				sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
				while (data[i] == '\r' || data[i] == '\n') { i++; }
			}
		}
	}

        triple = (sextet_a << 3 * 6)
                 + (sextet_b << 2 * 6)
                 + (sextet_c << 1 * 6)
                 + (sextet_d << 0 * 6);

        decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }
    decoded_data[j] = '\0';
    if (data[i-1] == '=')
    {
      j--;
      if (data[i-2] == '=')
      {
        j--;
      }
    }
    output_length = j;

    return decoded_data;
}


