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
#include <sys/select.h>
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


int main(int argc,char **argv){
    char buff[BUFFER_SIZE] = "Random message123";
    int fd, sent_bytes;
    double start_time,end_time;
    struct sockaddr_in to;
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

    clock = CLOCK_PROCESS_CPUTIME_ID;

    sent_bytes = send_packet(fd,to,ip,buff);
    start_time = get_current_secs(clock);

    handle_reply(fd,sent_bytes,buff);
    end_time = get_current_secs(clock);

    printf("The RTT was: %f seconds.\n", (double) (end_time - start_time));
}