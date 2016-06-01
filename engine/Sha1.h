
using namespace std;

#ifndef _SHA1_H_
#define _SHA1_H_


class SHA1
{

public:

	SHA1(){
		Reset();
	}
        virtual ~SHA1(){}

      
        //  Re-initialize the class

        void Reset(){
		Length_Low          = 0;
		Length_High         = 0;
		Message_Block_Index = 0;

		H[0]        = 0x67452301;
		H[1]        = 0xEFCDAB89;
		H[2]        = 0x98BADCFE;
		H[3]        = 0x10325476;
		H[4]        = 0xC3D2E1F0;

		Computed    = false;
		Corrupted   = false;
	}

        /*
         *  Returns the message digest
         */
        bool Result(unsigned *message_digest_array){
		int i;                                  // Counter

		if (Corrupted){
			return false;
		}

		if (!Computed){
			PadMessage();
			Computed = true;
		}

		for(i = 0; i < 5; i++){
			message_digest_array[i] = H[i];
		}

		return true;
	}
	static bool RunSHA1(string source, string *output){
		SHA1 sha;
		char result[41];
		unsigned message_digest[5];

		sha.Reset();
	
		sha.Input(source.c_str(), source.length());
		if(!sha.Result(message_digest)){
			return false;
		}

		sprintf(result, "%02X%02X%02X%02X%02X", message_digest[0], message_digest[1], message_digest[2], message_digest[3], message_digest[4]);

		output->append(result);

		sha.~SHA1();

		return true;
	}

        void Input( const unsigned char *message_array,unsigned length){
		if (!length){
			return;
		}

		if (Computed || Corrupted){
			Corrupted = true;
			return;
		}

		while(length-- && !Corrupted){
			Message_Block[Message_Block_Index++] = (*message_array & 0xFF);

			Length_Low += 8;
			Length_Low &= 0xFFFFFFFF;               // Force it to 32 bits
			if (Length_Low == 0){
				Length_High++;
				Length_High &= 0xFFFFFFFF;          // Force it to 32 bits
				if (Length_High == 0)    {
					Corrupted = true;               // Message is too long
				}
			}

			if (Message_Block_Index == 64){
				ProcessMessageBlock();
			}
			message_array++;
		}
	}

        void Input( const char  *message_array,unsigned length){

		Input((unsigned char *) message_array, length);
	}
        void Input(unsigned char message_element){

		Input(&message_element, 1);
	}
	
        void Input(char message_element){
		Input((unsigned char *) &message_element, 1);
	}
        SHA1& operator<<(const char *message_array){

		const char *p = message_array;

		while(*p){
			Input(*p);
			p++;
		}

		return *this;
	}
        SHA1& operator<<(const unsigned char *message_array){
		const unsigned char *p = message_array;

		while(*p){
			Input(*p);
			p++;
		}

		return *this;

	}
        SHA1& operator<<(const char message_element){
		Input((unsigned char *) &message_element, 1);

		return *this;

	}
        SHA1& operator<<(const unsigned char message_element){

		Input(&message_element, 1);

		return *this;
	}

private:

	void ProcessMessageBlock(){
		const unsigned K[] = {0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6};// Constants defined for SHA-1

		int t;                          // Loop counter
		unsigned temp;                       // Temporary word value
		unsigned W[80];                      // Word sequence
		unsigned A, B, C, D, E;              // Word buffers

		for(t = 0; t < 16; t++){
			W[t] = ((unsigned) Message_Block[t * 4]) << 24;
			W[t] |= ((unsigned) Message_Block[t * 4 + 1]) << 16;
			W[t] |= ((unsigned) Message_Block[t * 4 + 2]) << 8;
			W[t] |= ((unsigned) Message_Block[t * 4 + 3]);
		}

		for(t = 16; t < 80; t++){
			W[t] = CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
		}

		A = H[0];
		B = H[1];
		C = H[2];
		D = H[3];
		E = H[4];

		for(t = 0; t < 20; t++){
			temp = CircularShift(5,A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CircularShift(30,B);
			B = A;
			A = temp;
		}

		for(t = 20; t < 40; t++){
			temp = CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CircularShift(30,B);
			B = A;
			A = temp;
		}

		for(t = 40; t < 60; t++){
			temp = CircularShift(5,A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CircularShift(30,B);
			B = A;
			A = temp;
		}

		for(t = 60; t < 80; t++){
			temp = CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CircularShift(30,B);
			B = A;
			A = temp;
		}

		H[0] = (H[0] + A) & 0xFFFFFFFF;
		H[1] = (H[1] + B) & 0xFFFFFFFF;
		H[2] = (H[2] + C) & 0xFFFFFFFF;
		H[3] = (H[3] + D) & 0xFFFFFFFF;
		H[4] = (H[4] + E) & 0xFFFFFFFF;

		Message_Block_Index = 0;

	}

	void PadMessage(){

		if (Message_Block_Index > 55){
			Message_Block[Message_Block_Index++] = 0x80;

			while(Message_Block_Index < 64){
			    Message_Block[Message_Block_Index++] = 0;
			}

			ProcessMessageBlock();

			while(Message_Block_Index < 56){
				Message_Block[Message_Block_Index++] = 0;
			}
		}
		else{
			Message_Block[Message_Block_Index++] = 0x80;
			while(Message_Block_Index < 56){
				Message_Block[Message_Block_Index++] = 0;
			}
		}

		Message_Block[56] = (Length_High >> 24) & 0xFF;
		Message_Block[57] = (Length_High >> 16) & 0xFF;
		Message_Block[58] = (Length_High >> 8) & 0xFF;
		Message_Block[59] = (Length_High) & 0xFF;
		Message_Block[60] = (Length_Low >> 24) & 0xFF;
		Message_Block[61] = (Length_Low >> 16) & 0xFF;
		Message_Block[62] = (Length_Low >> 8) & 0xFF;
		Message_Block[63] = (Length_Low) & 0xFF;

		ProcessMessageBlock();
	}


	inline unsigned CircularShift(int bits, unsigned word){
		return ((word << bits) & 0xFFFFFFFF) | ((word & 0xFFFFFFFF) >> (32-bits));
	}

	unsigned H[5];                      // Message digest buffers

	unsigned Length_Low;                // Message length in bits
	unsigned Length_High;               // Message length in bits

	unsigned char Message_Block[64];    // 512-bit message blocks
	int Message_Block_Index;            // Index into message block array

	bool Computed;                      // Is the digest computed?
	bool Corrupted;                     // Is the message digest corruped?

};

#endif
