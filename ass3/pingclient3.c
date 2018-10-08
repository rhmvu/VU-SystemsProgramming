//
// Created by ruben on 8-9-18.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "pingutils.h"
#include "pingutils2.h"

#define BUFFER_SIZE 64

unsigned int breakloop = 0;


void print_status_with_counter(int reply_status,int reply_count,unsigned int count,double start_time, double end_time){
    if(reply_status == 0) {
        printf("Packet %u: lost.\n",count);
        return;
    }

    if(reply_count == count){
        printf("Packet %u: %f seconds.\n", count,(double) (end_time - start_time));
    }else{
        printf("Packet %u: wrong counter! Received %u instead of %u.\n",count,reply_status,count);
    }
}

void sigint_handler(int sigint) {
    if (breakloop == 0){
        breakloop=1;
        printf("SIGINT catched. Please wait to let the server close gracefully.\nTo close hard press Ctrl^C again.\n");
    }
    else{
        printf ("SIGINT occurred, exiting hard... please wait\n");
        exit(-1);
    }
}

int main(int argc,char **argv){
    char *buff;
    int fd, sent_bytes, reply_status,buffer_status,reply_count, scan_status, close_status;
    unsigned int count;
    double start_time,end_time;
    struct sockaddr_in to;
    struct in_addr *ip;
    clockid_t clock;


    if(argc<2){
        printf("Usage: pingclient3 <hostname>\n");
        return 1;
    }

    signal(SIGINT, sigint_handler );	// trap Ctrl^C signals

    //get ip of hostname and setup a socket
    ip = get_ip(argv[1]);
    fd = setup_socket();

    to.sin_family = AF_INET;
    to.sin_port  = htons(DEFAULT_PORT);
    to.sin_addr = *ip;

    buff = (char *) malloc(BUFFER_SIZE* sizeof(char));
    if(buff == NULL){
        perror("Can't allocate memory on heap");
        return 1;
    }

    clock = CLOCK_PROCESS_CPUTIME_ID;
    count = 1;

    while(breakloop != 1){
        //Put the packet number in the buffer
        buffer_status = sprintf(buff,"%d",count);
        if(buffer_status<0){
            perror("Cannot convert integer to char buffer");
            return 1;
        }
        //send buffer
        sent_bytes = send_packet(fd,to,ip,buff);
        start_time = get_current_secs(clock);

        reply_status = handle_reply_with_timeout(fd,sent_bytes,buff);

        //get packet number from received buffer (should be the same as the sent buffer when everything goes right)
        scan_status = sscanf(buff,"%d",&reply_count);
        if(scan_status<1){
            perror("Couldn't parse int from received bytes");
            exit(1);
        }
        end_time = get_current_secs(clock);

        print_status_with_counter(reply_status,reply_count,count,start_time,end_time);
        count++;
        sleep(1);
    }

    free(ip);
    close_status = close(fd);
    if(close_status<0){
        perror("Error closing socket");
        return 1;
    }
}