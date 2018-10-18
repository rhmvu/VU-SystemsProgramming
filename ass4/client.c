/* client.c
 *
 * part of the Systems Programming assignment
 * (c) Vrije Universiteit Amsterdam, 2005-2015. BSD License applies
 * author  : wdb -_at-_ few.vu.nl
 * contact : arno@cs.vu.nl
 * */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <dlfcn.h>
#include <netinet/in.h>

#include "library.h"
#include "audio.h"
#include "networking.h"
#include "protocol.h"


#define BUFSIZE 1024

static int breakloop = 0;	///< use this variable to stop your wait-loop. Occasionally check its value, !1 signals that the program should close

/// unimportant: the signal handler. This function gets called when Ctrl^C is pressed
void sigint_handler(int sigint)
{
	if (!breakloop){
		breakloop=1;
		printf("SIGINT catched. Please wait to let the client close gracefully.\nTo close hard press Ctrl^C again.\n");
	}
	else{
       		printf ("SIGINT occurred, exiting hard... please wait\n");
		exit(-1);
	}
}

int main (int argc, char *argv [])
{
	int server_fd, audio_fd, helo_status, control_status;
	int* sample_size;
	int* sample_rate;
	int* channels;
	client_filterfunc pfunc;
	struct in_addr *ip;
	struct sockaddr_in to;

	char* buffer = (char *) allocate_memory(BUFSIZE);

	sample_size = (int *) allocate_memory(sizeof(int));
	sample_rate = (int *) allocate_memory(sizeof(int));
	channels = (int *) allocate_memory(sizeof(int));



	printf ("SysProg2006 network client\n");
	printf ("handed in by Ruben van der Ham, 2592271\n");
	
	signal( SIGINT, sigint_handler );	// trap Ctrl^C signals
	
	// parse arguments
	if (argc < 3){
		printf ("error : called with incorrect number of parameters\nusage : %s <server_name/IP> <filename> [<filter> [filter_options]]]\n", argv[0]) ;
		return -1;
	}


	if(argc<2){
		printf("Usage: pingclient1 <hostname>\n");
		return 1;
	}
	//get ip of hostname and setup a socket
	ip = get_ip(argv[1]);
	server_fd = setup_socket();

	to.sin_family = AF_INET;
	to.sin_port  = htons(DEFAULT_PORT);
	to.sin_addr = *ip;

	//try to connect to the server 3 times
	for (int i = 0; i < 3; ++i) {
		helo_status = setup_helo_connection(server_fd,&to);
		if(helo_status == 1){
			break;
		}
		sleep(1);
	}
	//0 on timeout, -1 on error, 1 on success, 2 on succesfull connection reset
	if(helo_status == 2){
		printf("Connection reset during helo\n");
		return -1;
	}
	if(helo_status == 0){
		printf("Timeout couldn't reach server\n");
		return -1;
	}
	if(helo_status == -1){
		printf("Error sending HELO message to server\n");
		return -1;
	}

	//printf("HELO SUCCESS");
	control_status = setup_control_message(server_fd,&to,argv[2],argv[3]);
	if(control_status == -1){
		initiate_rst(server_fd,&to);
		return -1;
	}
	if(control_status == 2){
		printf("Server has reset the connection\n");
		return -1;
	}
	if(control_status == 0){
		printf("Control timeout\n");
		return -1;
	}

	printf("CONTROL SUCCESS\n");

	// TO IMPLEMENT
	// send the requested filename and library information to the server
	// and wait for an acknowledgement. Or fail if the server returns an errorcode	
	{
		*sample_size = 4;
		*sample_rate = 44100;
		*channels = 2;
	}
	
	// open output
	audio_fd = aud_writeinit((int) *sample_rate,(int) *sample_size,(int) *channels);
	if (audio_fd < 0){
		printf("error: unable to open audio output.\n");
		return -1;
	}
	
	// open the library on the clientside if one is requested
	if (argv[3] && strcmp(argv[3],"")){
		// try to open the library, if one is requested
		pfunc = NULL;
		if (!pfunc){
			printf("failed to open the requested library. breaking hard\n");
			return -1;
		}
		printf("opened libraryfile %s\n",argv[3]);
	}
	else{
		pfunc = NULL;
		printf("not using a filter\n");
	}
	
	// start receiving data
	{
		int bytesread, bytesmod;
		char *modbuffer;
		
		bytesread = read(server_fd, buffer, BUFSIZE);
		while (bytesread > 0){
			// edit data in-place. Not necessarily the best option
			if (pfunc)
				modbuffer = pfunc(buffer,bytesread,&bytesmod); 
			write(audio_fd, modbuffer, bytesmod);
			bytesread = read(server_fd, buffer, BUFSIZE);
		}
	}

	free(channels);
	free(sample_rate);
	free(sample_size);
	free(ip);
	free(buffer);

	if (audio_fd >= 0)	
		close(audio_fd);
	if (server_fd >= 0)
		close(server_fd);
	
	return 0 ;
}

