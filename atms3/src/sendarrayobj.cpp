#include "../h/sendarrayobj.h"

//pthread_mutex_t mutex_sendArray = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex_count = PTHREAD_MUTEX_INITIALIZER;

/**
 * sendArrayObj::sendArrayObj() creates sendArrayObj object.
 *
 * @param size_arg specifies size of the message queue. If number of elemenst will reach this number the first elemens will be removed from queue.
 */
sendArrayObj::sendArrayObj( int size_arg){	
	this->size = size;
	//default value validation
	if (size_arg <= 0 ){
		size = 50000;
	} else {
		size = size_arg; 
        }
	//size = 1000;
	toEnqueue = 0;
	toDequeue = 0;
	totalEnqueued = 0;
	totalDequeued = 0;
	lastDequeuedInd = -1;
	arr = (string**)calloc(size*sizeof(string*),1);//alloc number of pointers
	//lengths = (int*)calloc(size*sizeof(int),1);//alloc number of lengths
}

/**
 * sendArrayObj::~sendArrayObj() object destructor.
 */
sendArrayObj::~sendArrayObj()
{
	string * str;
	while (totalEnqueued-totalDequeued)
	{
		str = dequeue();
		delete str;
	}
	free(arr);
	//free(lengths);
}

/**
 * sendArrayObj::enqueue() function enqueues message for Engine
 * 
 * @param str is a binary string to enqueue
 * @return No return value
 */

void sendArrayObj::enqueue(string * str){
	if (arr[toEnqueue]!= NULL){
		delete arr[toEnqueue];
		toDequeue = (toDequeue+1)%size;
		totalDequeued++;
	}
	arr[toEnqueue] = str;
	toEnqueue = (toEnqueue+1)%size;
	totalEnqueued++;
}

/**
 * sendArrayObj::dequeue() functions dequeues oldest element from queue.
 * 
 * @return str is string to dequeue, must be later free'd
 */
string * sendArrayObj::dequeue(){
	string * ans = arr[toDequeue];
	lastDequeuedInd = ans->size();
	arr[toDequeue] = NULL;
	toDequeue = (toDequeue+1)%size;
	totalDequeued++;
	return ans;	
}


/**
 * sendArrayObj::count() function returns number of messages in the queue
 * 
 * @return number of elemenst in queue
 */
int sendArrayObj::count(){
	int ans = totalEnqueued-totalDequeued; 
	return ans;
}


int sendArrayObj::getDequeuedSize(){	
	return lastDequeuedInd;
}

