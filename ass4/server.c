/* server.c
 *
 * part of the Systems Programming assignment
 * (c) Vrije Universiteit Amsterdam, 2005-2015. BSD License applies
 * author  : wdb -_at-_ few.vu.nl
 * contact : arno@cs.vu.nl
 * */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <signal.h>

#include "library.h"
#include "audio.h"
#include "networking.h"
#include "protocol.h"

/// a define used for the copy buffer in stream_data(...)
#define BUFSIZE 1024
#define DEFAULT_PORT 2012

static int breakloop = 0;	///< use this variable to stop your wait-loop. Occasionally check its value, !1 signals that the program should close

/// stream data to a client. 
///
/// This is an example function; you do *not* have to use this and can choose a different flow of control
///
/// @param fd an opened file descriptor for reading and writing
/// @return returns 0 on success or a negative errorcode on failure
int stream_data(int client_fd)
{
	int data_fd;
	int channels, sample_size, sample_rate, control_status;
	server_filterfunc pfunc;
	char *datafile, *libfile;
	char buffer[BUFSIZE];
	char *datafile = (char*) malloc(sizeof(char)*FILENAME_SIZE);
	char *libfile = (char*) malloc(sizeof(char)*FILENAME_SIZE);
	if(datafile == NULL || libfile==NULL){
		perror("Can't allocate memory for control data");
		return -1;
	}

	// TO IMPLEMENT
	// receive a control packet from the client 
	// containing at the least the name of the file to stream and the library to use
    control_status = handle_control_message(fd,&datafile,&libfile);
    if(control_status <0){
        printf("can't parse control packet");
        return -1;
    }
	if(reply_status <0){
    	perror("Error receiving control packet");
		return -1;
	}

		/*datafile = strdup("example.wav");
		libfile = NULL;*/
	
	// open input
	data_fd = aud_readinit(datafile, &sample_rate, &sample_size, &channels);
	if (data_fd < 0){
		printf("failed to open datafile %s, skipping request\n",datafile);
		send_client("FAIL:datafile")
		return -1;
	}
	printf("opened datafile %s\n",datafile);

	// optionally open a library
	if (libfile){
		// try to open the library, if one is requested
		pfunc = NULL;
		if (!pfunc){
			printf("failed to open the requested library. breaking hard\n");
			return -1;
		}
		printf("opened libraryfile %s\n",libfile);
	}
	else{
		pfunc = NULL;
		printf("not using a filter\n");
	}
	
	// TO IMPLEMENT : optionally return an error code to the client if initialization went wrong
	
	// start streaming
	{
		int bytesread, bytesmod;
		
		bytesread = read(data_fd, buffer, BUFSIZE);
		while (bytesread > 0){
			// you might also want to check that the client is still active, whether it wants resends, etc..
			
			// edit data in-place. Not necessarily the best option
			if (pfunc)
				bytesmod = pfunc(buffer,bytesread); 
			write(client_fd, buffer, bytesmod);
			bytesread = read(data_fd, buffer, BUFSIZE);
		}
	}

	// TO IMPLEMENT : optionally close the connection gracefully 	
	
	if (client_fd >= 0)
		close(client_fd);
	if (data_fd >= 0)
		close(data_fd);
	if (datafile)
		free(datafile);
	if (libfile)
		free(libfile);
	
	return 0;
}

/// unimportant: the signal handler. This function gets called when Ctrl^C is pressed
void sigint_handler(int sigint)
{
	if (!breakloop){
		breakloop=1;
		printf("SIGINT catched. Please wait to let the server close gracefully.\nTo close hard press Ctrl^C again.\n");
	}
	else{
       		printf ("SIGINT occurred, exiting hard... please wait\n");
		exit(-1);
	}
}

/// the main loop, continuously waiting for clients
int main (int argc, char **argv)
{
	int msg_length, fd, close_status, stream_status, helo_status,rst_status;
	char buffer[BUFSIZE];
	struct sockaddr_in from;
	socklen_t from_len;
	from_len = sizeof(from);

	printf ("SysProg network server\n");
	printf ("handed in by Ruben van der Ham, 2592271\n");
	
	signal(SIGINT, sigint_handler );	// trap Ctrl^C signals

	fd = setup_server_socket(DEFAULT_PORT);

	if(fd<0){
		perror("Couln't create socket");
		exit(1);
	}
	
	while (!breakloop){
		helo_status = handle_helo_connection_setup(fd,from);
		if(helo_status == 0 || helo_status == -1){
			//on timeout ACK packet or error: reset connection
			for (int i = 0; i < 3 && rst_status !=1; ++i) {
				rst_status = initiate_rst(fd,from);
			}
		} else if(helo_status ==2){
			//on connection reset:
			continue;
		}


		stream_status = stream_data(fd,from,from_len);
		if(stream_status < 0){
			perror("Error streaming to %s", inet_ntoa(from.sin_addr));
			break;
		}
		// TO IMPLEMENT: 
		// 	wait for connections
		// 	when a client connects, start streaming data (see the stream_data(...) prototype above)
		sleep(1);
	}
	close_status = close(fd);
	if(close_status < 0) {
		perror("Couldn't close filedescriptor");
	}

	return 0;
}

