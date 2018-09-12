//
// Created by ruben on 8-9-18.
//
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#include "pingutils.h"

#define BUFFER_SIZE 18


int main(int argc,char **argv){
    char buff[BUFFER_SIZE] = "Random message123";
    int fd, sent_bytes,close_status;
    double start_time,end_time;
    struct sockaddr_in to;
    struct in_addr *ip;
    clockid_t clock;


    if(argc<2){
        printf("Usage: pingclient1 <hostname>\n");
        return 1;
    }
    //get ip of hostname and setup a socket
    ip = get_ip(argv[1]);
    fd = setup_socket();

    to.sin_family = AF_INET;
    to.sin_port  = htons(DEFAULT_PORT);
    to.sin_addr = *ip;
    free(ip);

    clock = CLOCK_PROCESS_CPUTIME_ID;

    sent_bytes = send_packet(fd,to,ip,buff);
    start_time = get_current_secs(clock);

    handle_reply(fd,sent_bytes,buff);
    end_time = get_current_secs(clock);

    close_status = close(fd);
    if(close_status<0){
        perror("Error closing socket");
        return 1;
    }

    printf("The RTT was: %f seconds.\n", (double) (end_time - start_time));
}