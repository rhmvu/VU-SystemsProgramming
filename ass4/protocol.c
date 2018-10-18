//
// Created by ruben on 29-9-18.
//

#include <netinet/in.h>
#include <string.h>
#include "protocol.h"
#include "networking.h"

extern const int CONTROL_FAIL_DATAFILE = 1;
extern const int CONTROL_FAIL_LIBRARY = 2;
extern const int CONTROL_FAIL_GENERIC = 3;
extern const char* PROT_HELO = "HELO";
extern const char* PROT_ACK = "ACK";
extern const char* PROT_RST = "RST";
extern const char* PROT_RST_ACK = "RST_ACK";

const char* TOKEN_DATAFILE = "datafile";
const char* TOKEN_LIBFILE = "libfile";

extern const int CONTROL_BUFSIZE =  2048;
extern const int HELO_BUFSIZE =  8;
extern const int FILENAME_SIZE =  512;
extern const int PROT_ACK_SIZE =  4;
extern const int PROT_RST_SIZE =  4;
extern const int PROT_RST_ACK_SIZE =  8;


/*
 * @return: 0 on timeout, -1 on error, 1 success
 */
void handle_control_message(int fd, char* datafile, char* libfile){
    int reply_status;
    char buffer[CONTROL_BUFSIZE];
    /*char **data;

    //Initialize charbuffers
    for (int i = 0; i < 2; ++i) {
        data[i] = (char*) malloc(TOKEN_SIZE);
        if(data[i] == NULL){
            perror("Can't allocate memory for control data");
            exit(1);
        }
    }*/


    reply_status = receive_packet_with_timeout(fd,CONTROL_BUFSIZE,&buffer); //buffer failure??
    if(receive_status == 0){
        //timeout occured
        return 0;
    }
    if(receive_status < 0){
        return -1;
    }



}

/*
 * @return: 0 on timeout, -1 on error, 1 on success, 2 on connection reset
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
    }else if(strncmp(buffer,PROT_RST,msg_length)){ //If client resets connection reply with RST_ACK
        reply_to_rst(fd,from);
        return 2;
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
        //timeout occurred
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


int reply_to_rst(fd,sockaddr_in *from){
    int sent_status = cket(fd,from,from->sin_addr,PROT_RST_ACK);
    if(sent_status < PROT_RST_ACK){
        return -1;
    }
    return 1;
}


/*
 *@return: 0 failed, 1 success
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
int send_message(int fd, struct sock_addr_in *to, char* buff){
    int sent_status,msg_length;
    char buffer[PROT_ACK_SIZE];

    sent_status = send_packet(fd,to,to->sin_addr,buff);


    msg_length = strnlen(buffer,PROT_ACK_SIZE); //max bufsize
    if(sent_status < msg_length){
        return -1;
    }


    //receive first ack
    receive_status = receive_packet_with_timeout(fd, PROT_ACK_SIZE,(struct sockaddr *) from,(char *) &buffer);
    if(receive_status == 0){
        //timeout occurred
        return 0;
    }
    if(receive_status < PROT_ACK_SIZE){
        return -1;
    }
    if(!strncmp(buffer,PROT_ACK,PROT_ACK_SIZE)){ //maybe msglength -1?
        return -1;
    }

    //send second ack
    sent_status = send_packet(fd,to,to->sin_addr,PROT_ACK);


    msg_length = strnlen(buffer,PROT_ACK_SIZE); //max bufsize
    if(sent_status < msg_length){
        return -1;
    }
    return 1;
}



/*
 * @return: 0 on timeout, -1 on error, 1 on success, 2 on RST, 3 on RST_ACK
 */
int receive_message(int fd, struct sockaddr *from, char* buff){
    int receive_status, msg_length;
    char ack_buffer[PROT_ACK_SIZE];

    receive_status = receive_packet_with_timeout(fd, CONTROL_BUFSIZE,(struct sockaddr *) from,(char *) buff);
    if(receive_status == 0){
        //timeout occurred
        return 0;
    }
    if(strncmp(buffer,PROT_RST,PROT_RST_SIZE)){ //maybe msglength -1?
        return 2;
    }
    if(strncmp(buffer,PROT_RST,PROT_RST_ACK_SIZE)){ //maybe msglength -1?
        return 3;
    }

    //send first ack
    sent_status = send_packet(fd,from,from->sin_addr,PROT_ACK);


    msg_length = strnlen(buffer,PROT_ACK_SIZE); //max bufsize
    if(sent_status < msg_length){
        return -1;
    }

    //receive second ack
    receive_status = receive_packet_with_timeout(fd, CONTROL_BUFSIZE,(struct sockaddr *) from,&ack_buffer);
    if(receive_status == 0){
        //timeout occurred
        return 0;
    }
    if(strncmp(buffer,PROT_ACK,PROT_ACK_SIZE)) { //maybe msglength -1?
        return -1;
    }

    return 1;
}


/*
void
 strsep(&str, ","){}
*/


//https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c