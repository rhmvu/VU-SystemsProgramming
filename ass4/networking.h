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

int send_packet(int fd,struct sockaddr_in to, struct in_addr *ip,char *buff);
int handle_reply(int fd,int max_reply_length, char *buff);


#endif //SYSTEMSPROGRAMMING_NETWORKING_H
