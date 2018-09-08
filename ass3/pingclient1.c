//
// Created by ruben on 8-9-18.
//
//
// Created by ruben on 8-9-18.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#define DEFAULT_PORT 2012
#define BUFFER_SIZE 18
#define NANO_OFFSET 1000000000



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
    return (struct in_addr*) addrp;

}

void send_packet(int fd,struct sockaddr_in to,socklen_t to_len, struct in_addr *ip,clockid_t clock){
    int msg_length,sent_length,received_length, time_status;
    char buff[BUFFER_SIZE] = "Random message123";

    struct sockaddr_in from;
    socklen_t from_len;
    struct timespec time;
    double start_time,end_time;

    msg_length = sizeof(char)*strlen(buff)+1;

    sent_length = sendto(fd, buff, msg_length, 0,(
            struct sockaddr *) &to, to_len);
    time_status = clock_gettime(clock,&time);
    if(time_status<0){
        perror("Error getting time after sent packet");
        exit(1);
    }
    start_time = (double) time.tv_nsec/NANO_OFFSET;

    if(sent_length!=msg_length){
        printf("sent: %d, msglength: %d\n",sent_length,msg_length);
        perror("Error sending bytes");
        exit(1);
    }

    received_length =  recvfrom(fd, buff, sent_length, 0,(struct sockaddr *) &from, &from_len);
    time_status = clock_gettime(clock,&time);
    if(time_status<0){
        perror("Error getting time after received packet");
        exit(1);
    }
    if(received_length < 0){
        perror("Error retrieving bytes from UDP packet");
        exit(1);
    }
    end_time = (double) time.tv_nsec/NANO_OFFSET;
    printf("The RTT was: %f seconds.\n", (double) (end_time-start_time));
}


int main(int argc,char **argv){
    int fd;
    struct sockaddr_in to;
    socklen_t to_len;
    struct in_addr *ip;
    clockid_t clock;
    

    if(argc<2){
        printf("Usage: pingclient1 <hostname>\n");
        return 1;
    }
    //get ip of hostname and setup a socket
    ip = get_ip(argv[1]);
    fd = setup_socket();

    to.sin_family = AF_INET;
    to.sin_port  = htons(DEFAULT_PORT);
    to.sin_addr = *ip;
    free(ip);

    to_len = sizeof(to);
    clock = CLOCK_PROCESS_CPUTIME_ID;


    send_packet(fd,to,to_len,ip,clock);
}
