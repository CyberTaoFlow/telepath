#include "../h/backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <syslog.h>

/**
 * open_write_pipe() function opens the pipe through which we communicate with the engine.
 * The pipe is opened in non blocking mode.
 * 
 * @return No return values.
 */
int Backend::open_write_pipe()
{
	pipe_fd = 0;
	if(access(this->pipename, F_OK) == -1)//checking if file(for pipe) exists.
	{
		this->res=mkfifo(this->pipename,0777);//If not, create it.
		if(this->res!=0)
		{
			fprintf(stderr,"Could not create fifo %s(O_WRONLY)\n",this->pipename);
			exit(EXIT_FAILURE);
		}
	}

	pipe_fd=open(this->pipename,O_RDWR);//opening pipe
	fcntl(pipe_fd, F_SETFL, fcntl(pipe_fd, F_GETFL) | O_NONBLOCK);
	return pipe_fd;

	if (fcntl(pipe_fd, F_SETFL, fcntl(pipe_fd, F_GETFL) | O_NONBLOCK) < 0){	//make socket to non-blocking mode
		
	}
	else{
		syslog(LOG_INFO,"Failed to set the Engine socket Non-Blocking");
		printf("Failed to set the Engine socket Non-Blocking\n");
		
	}
	printf("Process %d result %d\n", getpid(),pipe_fd);
	return pipe_fd;
}


Backend::~Backend()
{
	if (pipe_fd!=0)
		close(pipe_fd);	
	unlink(pipename);
	printf("Pipe closed.\n");
	delete [] pipename;
}

/**
 * Backend::Backend() object constructor
 *
 * @param name points to engine pipe.
 */
Backend::Backend(char *name)
{

	this->pipename = new char[strlen(name)+1];
	strncpy(this->pipename,name,strlen(name));
	this->pipename[strlen(name)]='\0';
}
