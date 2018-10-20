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


#define BUFSIZE 4096

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

int connect_to_server(int server_fd,struct sockaddr_in *to, int* sample_rate, int* sample_size, int* channels,char* audiofile, char* libfile){

    int helo_status, control_status;
    //try to connect to the server 3 times
    for (int i = 0; i < 3; ++i) {
        helo_status = setup_helo_connection(server_fd, to);
        if (helo_status == 1) {
            break;
        }
        sleep(1);
    }
    //0 on timeout, -1 on error, 1 on success, 2 on succesfull connection reset
    if (helo_status == 2) {
        printf("Connection reset during helo\n");
        return -1;
    }
    if (helo_status == 0) {
        printf("Timeout couldn't reach server\n");
        return -1;
    }
    if (helo_status == -1) {
        printf("Error sending HELO message to server\n");
        return -1;
    }

    {
        *sample_size = 4;
        *sample_rate = 44100;
        *channels = 2;
    }
    //printf("HELO SUCCESS");
    control_status = setup_control_message(server_fd, to, audiofile, libfile,sample_size,sample_rate,channels);
    if (control_status == -1) {
        initiate_rst(server_fd, to);
        return -1;
    }
    if (control_status == 2) {
        printf("Server has reset the connection\n");
        return -1;
    }
    if (control_status == 0) {
        printf("Control timeout\n");
        return -1;
    }

    /* printf("CONTROL SUCCESS\n");
     printf("size:%d\nrate:%d:chanells:%d\n\n",*sample_size,*sample_rate,*channels);*/
    return 1;
}

void free_audio_memory(char* buffer, char* modbuffer,int audio_fd, void* mylib){
    free(buffer);
    if (modbuffer != NULL && modbuffer != buffer&& !mylib) {
        free(modbuffer);
    }
    if (audio_fd >= 0) {
        printf("Closing audio descriptor\n");
        close(audio_fd);
    }
    if(mylib){
        dlclose(mylib);
    }
}

int play_audio(int server_fd,struct sockaddr_in *to,int* sample_rate, int* sample_size, int* channels,char* libfile){
    int audio_fd;
    void *mylib = 0;
    client_filterfunc pfunc;
    char *buffer = (char *) allocate_memory(BUFSIZE);
    // open output
    audio_fd = aud_writeinit((int) *sample_rate, (int) *sample_size, (int) *channels);
    if (audio_fd < 0) {
        printf("error: unable to open audio output.\n");
        free_audio_memory(buffer, NULL,audio_fd, mylib);
        return -1;
    }

    // open the library on the clientside if one is requested
    if (libfile && strcmp(libfile, "")) {
        mylib = dlopen(libfile, RTLD_NOW);

        // try to open the library, if one is requested
        pfunc = NULL;
        //open lib
        pfunc = dlsym(mylib, "decode");


        if (!pfunc) {
            printf("failed to open the requested library. breaking hard\n");
            free_audio_memory(buffer, NULL,audio_fd, mylib);
            return -1;
        }
        printf("opened libraryfile %s\n", libfile);
    } else {
        pfunc = NULL;
        printf("not using a filter\n");
    }


    // start receiving data

    //int mod;
    int wait_time = (int)(*sample_rate/ *channels);
    int bytesmod = 0;
    int bytesread = 0;
    int packet_count = 0;
    char *modbuffer = 0;
    if (pfunc) {
        modbuffer = allocate_memory(BUFSIZE);
    }
    int mod;

    bytesread = receive_message(server_fd, to, buffer, BUFSIZE);
    while (bytesread > 0 && !breakloop) {
        packet_count++;
        mod = packet_count%5;

        write(1,"\r               ",14);
        write(1,"\rPlaying.....",9+mod);


        //printf("read %d bytes, Audio status %d\n", bytesread,audio_status);
        // edit data in-place. Not necessarily the best option
        if (pfunc) {
            modbuffer = pfunc(buffer, bytesread, &bytesmod);
        } else {
            modbuffer = buffer;
            bytesmod = bytesread;
        }
        usleep(wait_time);

        write(audio_fd, modbuffer, bytesmod);
        for (int i = 0; i < 3; ++i) {
            bytesread = receive_message(server_fd, to, buffer, BUFSIZE);
            if(bytesread > 0){
                break;
            }
            if(bytesread == -2){
                reply_to_rst(server_fd,to);
                break;
            }
            if(bytesread == -3){
                free_audio_memory(buffer, modbuffer,audio_fd, mylib);
                return -1;
            }
        }
    }

    if(breakloop){
        initiate_rst(server_fd,to);
    } else {
        printf("\nDone streaming!\n");
    }

    free_audio_memory(buffer, modbuffer,audio_fd, mylib);
    return 1;
}


int main (int argc, char *argv []) {
    int server_fd;
    int *sample_size;
    int *sample_rate;
    int *channels;
    int status = -1;
    struct in_addr *ip;
    struct sockaddr_in *to = allocate_memory(sizeof(struct sockaddr_in));

    sample_size = (int *) allocate_memory(sizeof(int));
    sample_rate = (int *) allocate_memory(sizeof(int));
    channels = (int *) allocate_memory(sizeof(int));


    printf("SysProg2006 network client\n");
    printf("handed in by Ruben van der Ham, 2592271\n\n\n");

    signal(SIGINT, sigint_handler);    // trap Ctrl^C signals

    // parse arguments
    if (argc < 3) {
        printf("error : called with incorrect number of parameters\nusage : %s <server_name/IP> <filename> [<filter> [filter_options]]]\n",
               argv[0]);
        return -1;
    }


    if (argc < 2) {
        printf("Usage: pingclient1 <hostname>\n");
        return 1;
    }
    //get ip of hostname and setup a socket
    ip = get_ip(argv[1]);
    server_fd = setup_socket();

    to->sin_family = AF_INET;
    to->sin_port = htons(DEFAULT_PORT);
    to->sin_addr = *ip;

    if(connect_to_server(server_fd,to,sample_rate,sample_size,channels,argv[2],argv[3]) == 1){
        if(play_audio(server_fd,to,sample_rate,sample_size,channels,argv[3])==1){
            status = 0;
        }
    }

    free(channels);
    free(sample_rate);
    free(sample_size);
    free(ip);
    free(to);

    if (server_fd >= 0)
        close(server_fd);

    return status ;
}