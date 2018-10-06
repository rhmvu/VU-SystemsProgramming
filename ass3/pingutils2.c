//
// Created by ruben on 12-9-18.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <sys/select.h>
#include "pingutils2.h"


int handle_reply_with_timeout(int fd, int sent_length, char *buff){
    struct sockaddr_in from;
    socklen_t from_len;
    struct timeval timeout;
    fd_set read_set;
    int sel,received_length;
    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000; //timeout 0.5 seconds
    from_len = sizeof(from);

    //wait for the reply (until reply or timeout)
    sel = select(fd+1, &read_set, NULL, NULL, &timeout);
    if (sel<0) {
        perror("Error");
        exit(1);
    }
    if (sel==0) {
        return 0;
    }

    if (FD_ISSET(fd,&read_set)) {
        received_length = recvfrom(fd, buff, sent_length, 0, (struct sockaddr *) &from, &from_len);
        if (received_length < 0) {
            perror("Error retrieving bytes from UDP packet");
            exit(1);
        }
    }else{
        perror("File descriptor is not set as readable, though no timeout occurred");
        exit(1);
    }
    return 1;
}



void print_status(int reply_status,double start_time, double end_time){
    if(reply_status) {
        printf("The RTT was: %f seconds.\n", (double) (end_time - start_time));
    }else{
        printf("The packet was lost.\n");
    }
}
