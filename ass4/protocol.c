//
// Created by ruben on 29-9-18.
//

#include <netinet/in.h>
#include <string.h>
#include "protocol.h"
#include "networking.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

const int CONTROL_FAIL_DATAFILE = 1;
const int CONTROL_FAIL_LIBRARY = 2;
const int CONTROL_FAIL_GENERIC = 3;
const char* TOKEN_DELIMITER = ",";
char* PROT_HELO = "HELO";
char* PROT_ACK = "ACK";
char* PROT_RST = "RST";
char* PROT_RST_ACK = "RST_ACK";

const char* TOKEN_DATAFILE = "datafile";
const char* TOKEN_LIBFILE = "libfile";

const int CONTROL_BUFSIZE =  2048;
const int HELO_BUFSIZE =  8;
const int FILENAME_SIZE =  512;
const int PROT_HELO_SIZE =  5;
const int PROT_ACK_SIZE =  4;
const int PROT_RST_SIZE =  4;
const int PROT_RST_ACK_SIZE =  8;


/*
 * @return: 0 on timeout, -1 on error, 1 success
 */
int handle_control_message(int fd, struct sockaddr_in *from,char* datafile, char* libfile){
    int receive_status, tokenize_status;
    char* buffer = malloc(CONTROL_BUFSIZE);
    if(buffer == 0){
        perror("Cant allocate memroy");
        exit(1);
    }

    /*char **data;

    //Initialize charbuffers
    for (int i = 0; i < 2; ++i) {
        data[i] = (char*) malloc(TOKEN_SIZE);
        if(data[i] == NULL){
            perror("Can't allocate memory for control data");
            exit(1);
        }
    }*/


    receive_status = receive_message(fd,from,buffer,CONTROL_BUFSIZE); //buffer failure??
    if(receive_status == 0){
        //timeout occured
        return 0;
    }
    if(receive_status < 0){
        return -1;
    }
    if(receive_status == 2){
        reply_to_rst(fd,from);
        return -1;
    }


    tokenize_status = tokenize_control_message(buffer,datafile,libfile);
    if(tokenize_status <0){
        printf("Tokenize failed");
        return -1;
    }
    free(buffer);

    return 1;
}

/*
 * @return: 0 on timeout, -1 on error, 1 on success, 2 on connection reset
 */
int handle_helo_connection_setup(int fd, struct sockaddr_in *from){
    int receive_status, sent_status;
    char buffer[HELO_BUFSIZE];

    receive_status = receive_message(fd, from,(char *) &buffer, HELO_BUFSIZE);
    if(receive_status == 0){
       //timeout occured
        return 0;
    }
    if(receive_status < 0){
        return -1;
    }
    if(receive_status == 2){
        reply_to_rst(fd,from);
    }

    if(!strncmp(buffer,PROT_HELO,HELO_BUFSIZE)){ //maybe msglength -1?
        return -1;
    }

    printf("Host %s port %d: %s\n", inet_ntoa(from->sin_addr), ntohs(from->sin_port), buffer);

    //send HELO back
    sent_status = send_message(fd,from,PROT_HELO,PROT_HELO_SIZE);

    if(sent_status == 0){
        //timeout occured
        return 0;
    }
    if(sent_status < 0){
        return -1;
    }
    if(sent_status == 2){
        reply_to_rst(fd,from);
        return 2;
    }



    if(!strncmp(buffer,PROT_ACK,PROT_ACK_SIZE)){ //maybe msglength -1?
        return -1;
    }
    return 1;
}

/*
 * @return: 0 on timeout, -1 on error, 1 on success
 */
int initiate_rst(int fd,struct sockaddr_in *from) {
    int receive_status;
    //char buffer[HELO_BUFSIZE];
    //int msg_length;
    //send RST
    send_packet(fd,*from,&from->sin_addr,PROT_RST,PROT_RST_SIZE);

    //receive RST_ACK
    receive_status = receive_message(fd, from,PROT_RST,PROT_RST_SIZE);
    if(receive_status == 0){
        //timeout occurred
        return 0;
    }
    if(receive_status != 3){
        return -1;
    }
    /*msg_length = strnlen(buffer,HELO_BUFSIZE);

    if(!strncmp(buffer,PROT_RST_ACK_,msg_length)){ //maybe msglength -1?
        return -1;
    }*/
    return 1;
}


/*
 * @return: -1 on error, 1 on success
 */

int reply_to_rst(int fd,struct sockaddr_in *from){
    int sent_status = send_message(fd,from,PROT_RST_ACK,PROT_RST_ACK_SIZE);
    if(sent_status  != 1){
        return -1;
    }
    return 1;
}


/*
 *@return: -1 failed, 1 success
 */
int tokenize_control_message(char* message,char* datafile, char* libfile){
    char *token;
    token = strsep(&message, TOKEN_DELIMITER);
    if(!strncmp(token,TOKEN_DATAFILE,FILENAME_SIZE)){
        return -1;
    }
    datafile = strsep(&message, TOKEN_DELIMITER);

    token = strsep(&message, TOKEN_DELIMITER);
    if(!strncmp(token,TOKEN_LIBFILE,FILENAME_SIZE)){
        return -1;
    }

    libfile =  strsep(&message, TOKEN_DELIMITER);

    if(libfile == NULL || datafile == NULL){
        return -1;
    }
    return 1;
}



/*
 * @return: 0 on timeout, -1 on error, 1 on success
 */
int send_message(int fd, struct sockaddr_in *to, char* buff, int buf_size){
    int sent_status, receive_status;
    char *ack_buffer = malloc(PROT_ACK_SIZE);
    if(ack_buffer == 0){
        perror("Cant allocate memroy");
        exit(1);
    }

    sent_status = send_packet(fd,*to,&to->sin_addr,buff, buf_size);

    if(sent_status != buf_size){
        return -1;
    }


    //receive first ack
    receive_status = receive_packet_with_timeout(fd, PROT_ACK_SIZE, to,ack_buffer);
    if(receive_status == 0){
        //timeout occurred
        return 0;
    }
    if(receive_status < PROT_ACK_SIZE){
        return -1;
    }
    if(!strncmp(ack_buffer,PROT_ACK,PROT_ACK_SIZE)){ //maybe msglength -1?
        return -1;
    }

    //send second ack
    sent_status = send_packet(fd, *to,&to->sin_addr,PROT_ACK,PROT_ACK_SIZE);

    if(sent_status != PROT_ACK_SIZE){
        return -1;
    }
    return 1;
}



/*
 * @return: 0 on timeout, -1 on error, 1 on success, 2 on RST, 3 on RST_ACK
 */
int receive_message(int fd, struct sockaddr_in *from, char* buff,int buf_size){
    int receive_status, sent_status;
    char ack_buffer[PROT_ACK_SIZE];

    receive_status = receive_packet_with_timeout(fd, buf_size,from,(char *) buff);
    if(receive_status == 0){
        //timeout occurred
        return 0;
    }
    if(strncmp(buff,PROT_RST,PROT_RST_SIZE)){ //maybe msglength -1?
        return 2;
    }
    if(strncmp(buff,PROT_RST_ACK,PROT_RST_ACK_SIZE)){ //maybe msglength -1?
        return 3;
    }

    //send first ack
    sent_status = send_packet(fd, *from,&from->sin_addr,PROT_ACK,PROT_ACK_SIZE);

    if(sent_status < PROT_ACK_SIZE){
        return -1;
    }

    //receive second ack
    receive_status = receive_packet_with_timeout(fd, PROT_ACK_SIZE, from,ack_buffer);
    if(receive_status == 0){
        //timeout occurred
        return -1;
    }
    if(strncmp(ack_buffer,PROT_ACK,PROT_ACK_SIZE)) { //maybe msglength -1?
        return -1;
    }
    return 1;
}


/*
void
 strsep(&str, ","){}
*/


//https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c