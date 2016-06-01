#include "config.h"
#include "base64.h"

int Base64Decoder::default_base64_table[256];
const std::string Base64Decoder::default_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int* Base64Decoder::InitBase64Table(const std::string& alphabet)
	{
	assert(alphabet.size() == 64);

	static bool default_table_initialized = false;

	if ( alphabet == default_alphabet && default_table_initialized )
		return default_base64_table;

	int* base64_table = 0;

	if ( alphabet == default_alphabet )
		{
		base64_table = default_base64_table;
		default_table_initialized = true;
		}
	else
		base64_table = new int[256];

	int i;
	for ( i = 0; i < 256; ++i )
		base64_table[i] = -1;

	for ( i = 0; i < 26; ++i )
		{
		base64_table[int(alphabet[0 + i])] = i;
		base64_table[int(alphabet[26 + i])] = i + 26;
		}

	for ( i = 0; i < 10; ++i )
		base64_table[int(alphabet[52 + i])] = i + 52;

	// Casts to avoid compiler warnings.
	base64_table[int(alphabet[62])] = 62;
	base64_table[int(alphabet[63])] = 63;
	base64_table[int('=')] = 0;

	return base64_table;
	}

Base64Decoder::Base64Decoder(/* Analyzer* arg_analyzer, */ const std::string& alphabet)
	{
	base64_table = InitBase64Table(alphabet.size() ? alphabet : default_alphabet);
	base64_group_next = 0;
	base64_padding = base64_after_padding = 0;
	errored = 0;
	//analyzer = arg_analyzer;
	}

Base64Decoder::~Base64Decoder()
	{
	if ( base64_table != default_base64_table )
		{
		delete [] base64_table;
		}
	}

int Base64Decoder::Decode(int len, const char* data, int* pblen, char** pbuf)
	{
	int blen;
	char* buf;

	if ( ! pbuf )
		reporter->InternalError("nil pointer to decoding result buffer");

	if ( *pbuf )
		{
		buf = *pbuf;
		blen = *pblen;
		}
	else
		{
		// Estimate the maximal number of 3-byte groups needed,
		// plus 1 byte for the optional ending NUL.
		blen = int((len + base64_group_next + 3) / 4) * 3 + 1;
		*pbuf = buf = new char[blen];
		}

	int dlen = 0;

	while ( 1 )
		{
		if ( base64_group_next == 4 )
			{
			// For every group of 4 6-bit numbers,
			// write the decoded 3 bytes to the buffer.
			if ( base64_after_padding )
				{
				if ( ++errored == 1 )
					IllegalEncoding("extra base64 groups after '=' padding are ignored");
				base64_group_next = 0;
				continue;
				}

			int num_octets = 3 - base64_padding;

			if ( buf + num_octets > *pbuf + blen )
				break;

			uint32 bit32 =
				((base64_group[0] & 0x3f) << 18) |
				((base64_group[1] & 0x3f) << 12) |
				((base64_group[2] & 0x3f) << 6)  |
				((base64_group[3] & 0x3f));

			if ( --num_octets >= 0 )
				*buf++ = char((bit32 >> 16) & 0xff);

			if ( --num_octets >= 0 )
				*buf++ = char((bit32 >> 8) & 0xff);

			if ( --num_octets >= 0 )
				*buf++ = char((bit32) & 0xff);

			if ( base64_padding > 0 )
				base64_after_padding = 1;

			base64_group_next = 0;
			base64_padding = 0;
			}

		if ( dlen >= len )
			break;

		if ( data[dlen] == '=' )
			++base64_padding;

		int k = base64_table[(unsigned char) data[dlen]];
		if ( k >= 0 )
			base64_group[base64_group_next++] = k;
		else
			{
			if ( ++errored == 1 )
				IllegalEncoding(fmt("character %d ignored by Base64 decoding", (int) (data[dlen])));
			}

		++dlen;
		}

	*pblen = buf - *pbuf;
	return dlen;
	}

int Base64Decoder::Done(int* pblen, char** pbuf)
	{
	const char* padding = "===";

	if ( base64_group_next != 0 )
		{
		if ( base64_group_next < 4 )
			IllegalEncoding(fmt("incomplete base64 group, padding with %d bits of 0", (4-base64_group_next) * 6));
		Decode(4 - base64_group_next, padding, pblen, pbuf);
		return -1;
		}

	if ( pblen )
		*pblen = 0;

	return 0;
	}


static char encoding_table[256] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

static unsigned char decoding_table[256];

unsigned char *bro_base64_decode(const char *d, size_t input_length, size_t & output_length)
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

