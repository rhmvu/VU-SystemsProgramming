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

#include "pingutils.h"

const int DEFAULT_PORT = 2012;
const int NANO_OFFSET  = 1000000000;

int setup_socket(){
    int fd;
    fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (fd<0) {
        perror("Couln't create socket");
        exit(1);
    }
    return fd;
}


struct in_addr* get_ip(const char *name) {
    struct hostent *resolv;
    struct in_addr *addrp = malloc(sizeof(struct in_addr));
    resolv = gethostbyname(name);
    if (resolv==NULL) {
        perror("Address not found\n");
        exit(1);
    }
    addrp = (struct in_addr*) resolv->h_addr_list[0];
    return addrp;
}

void handle_reply(int fd, int sent_length, char *buff){
    struct sockaddr_in from;
    socklen_t from_len;
    int received_length;
    from_len = sizeof(from);

    received_length = recvfrom(fd, buff, sent_length, 0, (struct sockaddr *) &from, &from_len);
    if (received_length < 0) {
        perror("Error retrieving bytes from UDP packet");
        exit(1);
    }
}



int send_packet(int fd,struct sockaddr_in to, struct in_addr *ip,char *buff){
    int msg_length,sent_length;
    socklen_t to_len;
    to_len = sizeof(to);

    msg_length = sizeof(char)*strlen(buff)+1;

    //send the packet
    sent_length = sendto(fd, buff, msg_length, 0,(
            struct sockaddr *) &to, to_len);

    if(sent_length!=msg_length){
        perror("Error sending bytes");
        exit(1);
    }
    return sent_length;
}


double get_current_secs(clockid_t clock){
    struct timespec time;
    int time_status;
    time_status = clock_gettime(clock,&time);

    if(time_status<0){
        perror("Error getting time");
        exit(1);
    }
    return (double) time.tv_nsec/NANO_OFFSET;
}