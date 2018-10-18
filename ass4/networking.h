//
// Created by ruben on 29-9-18.
//

#ifndef SYSTEMSPROGRAMMING_NETWORKING_H
#define SYSTEMSPROGRAMMING_NETWORKING_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int setup_server_socket(int port);
int setup_socket();

int send_packet(int fd,struct sockaddr_in to, struct in_addr *ip,char *buff, int msg_length);
int receive_packet_with_timeout(int fd,int max_reply_length, struct sockaddr_in *from, char *buff);


#endif //SYSTEMSPROGRAMMING_NETWORKING_H
