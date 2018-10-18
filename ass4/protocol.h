//
// Created by ruben on 29-9-18.
//

#ifndef SYSTEMSPROGRAMMING_PROTOCOL_H
#define SYSTEMSPROGRAMMING_PROTOCOL_H

extern const int CONTROL_FAIL_DATAFILE;
extern const int CONTROL_FAIL_LIBRARY;
extern const int CONTROL_FAIL_GENERIC;



void handle_control_message(int fd, char* datafile, char* libfile);
int handle_helo_connection_setup(int fd, sockaddr_in *from);
int initiate_rst(fd,sockaddr_in *from);
int reply_to_rst(fd,sockaddr_in *from);
int tokenize_control_message(char* message,char* datafile, char* libfile);


int send_message(int fd, struct sock_addr_in *to, char* buff, int buf_size);
int receive_message(int fd, struct sockaddr *from, char* buff,int buf_size);



#endif //SYSTEMSPROGRAMMING_PROTOCOL_H
