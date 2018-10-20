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
const int DEFAULT_PORT = 2012;
const char* TOKEN_DELIMITER = ",";
char* PROT_HELO = "HELO";
char* PROT_ACK = "ACK";
//char* PROT_ACK = "RQST";
char* PROT_RST = "RST";
char* PROT_RST_ACK = "RST_ACK";
//char* PROT_RST_ACK = "RQST_ACK";
char* PROT_CTRL_ACK = "CTRL_ACK";

const char* TOKEN_DATAFILE = "datafile";
const char* TOKEN_LIBFILE = "libfile";

const int CONTROL_BUFSIZE =  1024;
const int HELO_BUFSIZE =  10;
const int TOKEN_SIZE =  512;
const int PROT_HELO_SIZE =  5;
const int PROT_ACK_SIZE =  4;
const int PROT_RQST_SIZE =  5;
const int PROT_RST_SIZE =  4;
const int PROT_RQST_ACK_SIZE =  9;
const int PROT_RST_ACK_SIZE =  8;
const int PROT_CTRL_ACK_SIZE =  9;




void send_request(int fd, struct sockaddr_in *from, int bytes){
    char buff[CONTROL_BUFSIZE];
    int size = snprintf(buff,CONTROL_BUFSIZE,"RQST: %d",bytes);
    printf("REQEUST: %d bytes",size-5);
    send_message(fd,from,buff,size);
}

// bytes set to -1 on timeout
int receive_request(int fd, struct sockaddr_in *from){
    int receive_status;
    char buff[CONTROL_BUFSIZE];
    for (int i = 0; i < 20; ++i) {
        receive_status = receive_message(fd,from,buff,CONTROL_BUFSIZE);
        if(receive_status == -1){
            return -1;
        }
        if(receive_status == -2){
            reply_to_rst(fd,from);
            return -2;
        }
        if(receive_status > 0){
            break;
        }
    }
    char rqstbuff[CONTROL_BUFSIZE];
    int bytebuff;
    sscanf(buff,"%s %d",rqstbuff,&bytebuff);
    if(!strncmp("RQST:",rqstbuff,6)){
        return -1;
    }
    return bytebuff;
}

/*
 * @return: 0 on timeout, -1 on error, 1 success
 */
int setup_control_message(int fd, struct sockaddr_in *from,char* datafile, char* libfile,int *sample_size,int *sample_rate,int *channels){
    int receive_status, length, sent_status,tokenize_status;
    char* buffer = allocate_memory(CONTROL_BUFSIZE);

    /*tokenize_status = snprintf(buffer,CONTROL_BUFSIZE,"%s%s%s%s%s%s%s",TOKEN_DATAFILE,TOKEN_DELIMITER,datafile,TOKEN_DELIMITER,TOKEN_LIBFILE,libfile);*/


     /*if(tokenize_status <0){
        printf("Serialization failed");
        return -1;
    }*/
    length = 0;
    length+=snprintf(buffer+length,TOKEN_SIZE,"%s",TOKEN_DATAFILE);
    length+=snprintf(buffer+length,TOKEN_SIZE,"%s",TOKEN_DELIMITER);
    length+=snprintf(buffer+length,TOKEN_SIZE,"%s",datafile);
    length+=snprintf(buffer+length,TOKEN_SIZE,"%s",TOKEN_DELIMITER);
    length+=snprintf(buffer+length,TOKEN_SIZE,"%s",TOKEN_LIBFILE);
    length+=snprintf(buffer+length,TOKEN_SIZE,"%s",TOKEN_DELIMITER);
    length+=snprintf(buffer+length,TOKEN_SIZE,"%s",libfile);
    //length+=snprintf(buffer+length,TOKEN_SIZE,);



    sent_status = send_message(fd,from,buffer,CONTROL_BUFSIZE);

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

    receive_status = receive_message(fd,from,buffer,CONTROL_BUFSIZE);
    if(receive_status == 0){
        //timeout occured
        return 0;
    }
    if(receive_status == -1){
        return -1;
    }
    if(receive_status == -2){
        reply_to_rst(fd,from);
        return 2;
    }

    if(strncmp(buffer,PROT_CTRL_ACK,PROT_CTRL_ACK_SIZE)  != 0){
        //receive_message(fd,from,buffer,CONTROL_BUFSIZE);
        /*//int buff_len = strnlen(buffer,CONTROL_BUFSIZE);
        //fwrite(buffer,1,buff_len,stdout);*/
        printf("%s",buffer);
        return -1;
    }

    //receive audio props
    receive_status = receive_message(fd,from,buffer,CONTROL_BUFSIZE);
    if(receive_status == 0){
        //timeout occured
        return 0;
    }
    if(receive_status == -1){
        return -1;
    }
    if(receive_status == -2){
        reply_to_rst(fd,from);
        return 2;
    }

    tokenize_status = tokenize_audio_props(buffer,sample_size,sample_rate,channels);
    if(tokenize_status <0){
        printf("Tokenize failed");
        return -1;
    }


    free(buffer);
    return 1;
}




/*
 * @return: 0 on timeout, -1 on error, 1 success
 */
int handle_control_message(int fd, struct sockaddr_in *from,char* datafile, char* libfile){
    int receive_status, tokenize_status;
    char* buffer = allocate_memory(CONTROL_BUFSIZE);

    receive_status = receive_message(fd,from,buffer,CONTROL_BUFSIZE); //buffer failure??
    if(receive_status == 0){
        //timeout occured
        return 0;
    }
    if(receive_status == -1){
        return -1;
    }
    if(receive_status == -2){
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
 * @return: 0 on timeout, -1 on error, 1 success, 2 on successful RST
 */
int confirm_control_message(int fd, struct sockaddr_in *from,int sample_size,int sample_rate, int channels){
    int sent_status,length;
    char* buffer = allocate_memory(CONTROL_BUFSIZE);
    for (int i = 0; i < 3; ++i) {
        sent_status = send_message(fd, from, PROT_CTRL_ACK, PROT_CTRL_ACK_SIZE);
        if(sent_status != 0){
            break;
        }
    }
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




    length = 0;
    length+=snprintf(buffer+length,TOKEN_SIZE,"%d",sample_size);
    length+=snprintf(buffer+length,TOKEN_SIZE,"%s",TOKEN_DELIMITER);
    length+=snprintf(buffer+length,TOKEN_SIZE,"%d",sample_rate);
    length+=snprintf(buffer+length,TOKEN_SIZE,"%s",TOKEN_DELIMITER);
    length+=snprintf(buffer+length,TOKEN_SIZE,"%d",channels);
    //printf("%s",buffer);
    sent_status = send_message(fd, from, buffer, length);
    free(buffer);
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

    return 1;
}



/*
 * @return: 0 on timeout, -1 on error, 1 on success, 2 on succesfull connection reset
 */
int setup_helo_connection(int fd, struct sockaddr_in * to){
    int receive_status, sent_status;
    char *buffer = allocate_memory(HELO_BUFSIZE);


    //send HELO
    sent_status = send_message(fd,to,PROT_HELO,PROT_HELO_SIZE);

    if(sent_status == 0){
        //timeout occured
        return 0;
    }
    if(sent_status < 0){
        return -1;
    }
    if(sent_status == 2){
        reply_to_rst(fd,to);
        return 2;
    }

    receive_status = receive_message(fd, to,(char *) buffer, HELO_BUFSIZE);
    if(receive_status == 0){
        //timeout occured
        return 0;
    }

    if(receive_status == -2){
        reply_to_rst(fd,to);
        return 2;
    }
    if(receive_status < 0){
        return -1;
    }

    //printf("RECEIVED: Host %s port %d: %s\n", inet_ntoa(to->sin_addr), ntohs(to->sin_port), buffer    );
    if(strncmp(buffer,PROT_HELO,HELO_BUFSIZE) != 0){ //maybe msglength -1?
        return -1;
    }


    /*if(strncmp(buffer,PROT_ACK,PROT_ACK_SIZE)){ //maybe msglength -1?
        return -1;
    }*/
    //printf("HELO setup up to: Host %s port %d\n", inet_ntoa(to->sin_addr), ntohs(to->sin_port));
    free(buffer);
    return 1;
}

/*
 * @return: 0 on timeout, -1 on error, 1 on success, 2 on connection reset
 */
int handle_helo_connection(int fd, struct sockaddr_in *from){
    int receive_status, sent_status;
    char buffer[HELO_BUFSIZE];

    receive_status = receive_message(fd, from,(char *) buffer, HELO_BUFSIZE);
    if(receive_status == 0){
       //timeout occurred
        return 0;
    }
    if(receive_status == -2){
        reply_to_rst(fd,from);
        return 2;
    }
    if(receive_status < 0){
        return -1;
    }
    if(strncmp(buffer,PROT_HELO,HELO_BUFSIZE) != 0 ){ //maybe msglength -1?
        return -1;
    }


    //send HELO back
    sent_status = send_message(fd,from,PROT_HELO,PROT_HELO_SIZE);
    if(sent_status == 0){
        //timeout occurred
        return 0;
    }
    if(sent_status < 0){
        return -1;
    }
    if(sent_status == 2){
        reply_to_rst(fd,from);
        return 2;
    }


    if(strncmp(buffer,PROT_HELO,PROT_HELO_SIZE) != 0){ //maybe msglength -1?
        return -1;
    }

    return 1;


}

/*
 * @return: 0 on timeout, -1 on error, 1 on success
 */
int initiate_rst(int fd,struct sockaddr_in *from) {
    int receive_status;
    char buffer[HELO_BUFSIZE];
    //send RST
    printf("INIT RST...\n");
    send_packet(fd,from,PROT_RST,PROT_RST_SIZE);

    //receive RST_ACK
    printf("receiving rstack..\n");
    receive_status = receive_message(fd, from,buffer,PROT_RST_ACK_SIZE);
    //printf("received rstack\n");
    if(receive_status == 0){
        //timeout occurred
        return 0;
    }
    if(receive_status == -2){
        printf("receiving -2");
        reply_to_rst(fd,from);
    }
    if(receive_status != -3){
        printf("RST INIT FAILED\n");
        return -1;
    }
    printf("SUCCESSFULL RST!\n");
    return 1;
}


/*
 * @return: -1 on error, 1 on success
 */

int reply_to_rst(int fd,struct sockaddr_in *from){
    printf("REPLYING TO RSTK ACK\n");
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
    //printf("%s\n\n",message);
    token = strsep(&message, TOKEN_DELIMITER);
    if(strncmp(token,TOKEN_DATAFILE,TOKEN_SIZE) != 0){
        return -1;
    }
    token = strsep(&message, TOKEN_DELIMITER);
    snprintf(datafile,TOKEN_SIZE,"%s",token);
    token = strsep(&message, TOKEN_DELIMITER);
    if(strncmp(token,TOKEN_LIBFILE,TOKEN_SIZE) != 0){
        return -1;
    }
    token =  strsep(&message, TOKEN_DELIMITER);
    snprintf(libfile,TOKEN_SIZE,"%s",token);
    if(libfile == NULL || datafile == NULL){
        return -1;
    }
    return 1;
}


/*
 *@return: -1 failed, 1 success
 */
int tokenize_audio_props(char *message,int *sample_size,int *sample_rate,int *channels){
    //char *token;
    //printf("%s\n\n",message);
    /*token = strsep(&message, TOKEN_DELIMITER);
    snscanf(sample_size,TOKEN_SIZE,"%d",token);
    token = strsep(&message, TOKEN_DELIMITER);
    snscanf(sample_rate,TOKEN_SIZE,"%d",token);
    token = strsep(&message, TOKEN_DELIMITER);*/
    sscanf(message,"%d,%d,%d",sample_size,sample_rate,channels);


    if(sample_rate == NULL || sample_size == NULL || channels == NULL){
        return -1;
    }
    return 1;
}


/*
 * @return: 0 on timeout, -1 on error, 1 on success
 */
int send_message(int fd, struct sockaddr_in *to, char* buff, int buf_size){
    int sent_status, receive_status;
    char *ack_buffer = allocate_memory(PROT_ACK_SIZE);

    sent_status = send_packet(fd,to,buff, buf_size);

    if(sent_status != buf_size){
        return -1;
    }

    //receive first ack
    receive_status = receive_packet_with_timeout(fd, PROT_ACK_SIZE, to,ack_buffer);
    if(receive_status == 0){
        //timeout occurred
        return 0;
    }
    if(!strncmp(buff,PROT_RST,PROT_RST_SIZE)){
        printf("RST DETECTED\n");
        return -2;
    }
    if(!strncmp(buff,PROT_RST_ACK,PROT_RST_ACK_SIZE)){
        printf("RST ACK DETECTED\n");
        return -3;
    }
    if(receive_status < PROT_ACK_SIZE){
        //printf("Received lees than prot ack, got %d expected %d, buffer %s", receive_status, PROT_ACK_SIZE,ack_buffer);
        return -1;
    }
    if(strncmp(ack_buffer,PROT_ACK,PROT_ACK_SIZE) != 0){
      //  printf("NO ACK IN BUFFER\n");
        return -1;
    }
    //printf("Sucessfully received ack1\n");
    //send second ack
    sent_status = send_packet(fd, to,PROT_ACK,PROT_ACK_SIZE);

    if(sent_status != PROT_ACK_SIZE){
        //printf("Sent size less than second prot ack\n");
        return -1;
    }
    free(ack_buffer);
    return 1;
}



/*
 * @return: 0 on timeout, -1 on error, amount of bytes received on success, -2 on RST, -3 on RST_ACK
 */
int receive_message(int fd, struct sockaddr_in *from, char* buff,int buf_size){
    int receive_status, sent_status, original_rcv_bytes;
    char ack_buffer[PROT_ACK_SIZE];

    original_rcv_bytes = 0;
    receive_status = receive_packet_with_timeout(fd, buf_size,from,(char *) buff);

    if(receive_status == 0){
        //timeout occurred
        return 0;
    }
    if(receive_status < 0){
        return -1;
    }

    if(!strncmp(buff,PROT_RST,PROT_RST_SIZE)){
        printf("RST DETECTED\n");
        return -2;
    }
    if(!strncmp(buff,PROT_RST_ACK,PROT_RST_ACK_SIZE)){
        printf("RST ACK DETECTED\n");
        return -3;
    }

    original_rcv_bytes = receive_status;

    //send first ack
    sent_status = send_packet(fd, from,PROT_ACK,PROT_ACK_SIZE);

    if(sent_status < PROT_ACK_SIZE){
        //printf("sent failure");
        return -1;
    }

    //receive second ack
    receive_status = receive_packet_with_timeout(fd, PROT_ACK_SIZE, from,ack_buffer);
    if(receive_status == 0){
        //timeout occurred
        return -1;
    }
    if(strncmp(ack_buffer,PROT_ACK,PROT_ACK_SIZE) != 0) { //maybe msglength -1?
        return -1;
    }

    return original_rcv_bytes;
}

void *allocate_memory(size_t size){
    void *status = malloc(size);
    if(!status){
        perror("Couldn't allocate memory");
        exit(1);
    }
    //memset(status,0,size);
    return status;
}


/*
void
 strsep(&str, ","){}
*/


//https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c