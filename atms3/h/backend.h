#ifndef BACKEND_H_
#define BACKEND_H_



//#define BUFFER_SIZE 1048567

/**
 * Backend class is used to store connection with engine.
 */
class Backend{
	public :
		// open backend connection
		int open_write_pipe();
		~Backend();
		Backend(char *name);
		char* pipename;
		int pipe_fd;
		//char buffer[BUFFER_SIZE];
		int res;
};













#endif /*BACKEND_H_*/
