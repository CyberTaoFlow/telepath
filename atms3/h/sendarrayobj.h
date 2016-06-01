
#ifndef __SENDARRAY_H_
#define __SENDARRAY_H_

using namespace std;
#include <string.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

/**
 * sendArrayObj class is used as a buffer when we have too many packets to send to engine
 *
 */

class sendArrayObj
{
	private:
		int size,toEnqueue,toDequeue,lastDequeuedInd;
		unsigned long long totalEnqueued,totalDequeued;		
		
	public:
		string ** arr;
		//int* lengths;
		sendArrayObj(int size);
		~sendArrayObj();
		void enqueue(string * str);
		string * dequeue();
		int count();
		int getDequeuedSize();
};



#endif
