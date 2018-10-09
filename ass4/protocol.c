//
// Created by ruben on 29-9-18.
//

#include <netinet/in.h>
#include "protocol.h"
#include "networking.h"

extern const int CONTROL_FAIL_DATAFILE = 1;
extern const int CONTROL_FAIL_LIBRARY = 2;
extern const char* PROT_HELO = "HELO";
extern const char* PROT_ACK = "ACK";
extern const char* PROT_RST = "RST";
extern const char* PROT_RST_ACK = "RST_ACK";

extern const int CONTROL_BUFSIZE =  2048;
extern const int HELO_BUFSIZE =  8;


/*
 * @return: 0 on timeout
 */
void handle_control_message(int fd, char* datafile, char* libfile){
    int reply_status;
    char buffer[CONTROL_BUFSIZE];
    reply_status =  receive_packet_with_timeout(fd,BUFSIZE,&buffer); //buffer failure??

}







/*
 * @return: 0 on timeout, -1 on error, 1 on success
 */
int handle_helo_connection_setup(int fd, sockaddr_in *from){
    int receive_status;
    char buffer[HELO_BUFSIZE];
    int msg_length;

    receive_status = receive_packet_with_timeout(fd, CONTROL_BUFSIZE,(struct sockaddr *) from,(char *) &buffer);
    if(receive_status == 0){
       //timeout occured
        return 0;
    }
    if(receive_status < 0){
        return -1;
    }
    msg_length = strnlen(buffer,HELO_BUFSIZE);

    if(!strncmp(buffer,PROT_HELO,msg_length)){ //maybe msglength -1?
        return -1;
    }

    printf("Host %s port %d: %s\n", msg_length, inet_ntoa(from->sin_addr), ntohs(from->sin_port), buff);

    //send HELO back
    send_packet(fd,from,from->sin_addr,PROT_HELO);

    //receive ACK because HELO received
    receive_status = receive_packet_with_timeout(fd, CONTROL_BUFSIZE,(struct sockaddr *) from,(char *) &buffer);
    if(receive_status == 0){
        //timeout occured
        return 0;
    }
    if(receive_status < 0){
        return -1;
    }
    if(!strncmp(buffer,PROT_ACK,msg_length)){ //maybe msglength -1?
        return -1;
    }
    return 1;
}

/*
 * @return: 0 on timeout, -1 on error, 1 on success
 */
int initiate_rst(fd,sockaddr_in *from) {
    int receive_status;
    char buffer[HELO_BUFSIZE];
    int msg_length;
    //send RST
    send_packet(fd,from,from->sin_addr,PROT_RST);

    //receive RST_ACK
    receive_status = receive_packet_with_timeout(fd, CONTROL_BUFSIZE,(struct sockaddr *) from,(char *) &buffer);
    if(receive_status == 0){
        //timeout occured
        return 0;
    }
    if(receive_status < 0){
        return -1;
    }
    msg_length = strnlen(buffer,HELO_BUFSIZE);

    if(!strncmp(buffer,PROT_RST_ACK_,msg_length)){ //maybe msglength -1?
        return -1;
    }
    return 1;
}

void reply_to_rst(fd,sockaddr_in *from){
    send_packet(fd,from,from->sin_addr,PROT_RST_ACK);
}
/*
void
 strsep(&str, ","){}
*/


//https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c