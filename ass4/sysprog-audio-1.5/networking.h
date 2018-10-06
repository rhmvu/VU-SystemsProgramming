//
// Created by ruben on 29-9-18.
//

#ifndef SYSTEMSPROGRAMMING_NETWORKING_H
#define SYSTEMSPROGRAMMING_NETWORKING_H


int setup_socket(int port);
int receive(int fd,int max_reply_length, char *buff);


#endif //SYSTEMSPROGRAMMING_NETWORKING_H
