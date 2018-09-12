//
// Created by ruben on 8-9-18.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "pingutils.h"
#include "pingutils2.h"

#define BUFFER_SIZE 64



void print_status_with_counter(int reply_status,int reply_count,unsigned int count,double start_time, double end_time){
    if(reply_status == 0) {
        printf("Packet %u: lost.\n",count);
    }

    if(reply_count == count){
        printf("Packet %u: %f seconds.\n", count,(double) (end_time - start_time));
    }else{
        printf("Packet %u: wrong counter! Received %u instead of %u.\n",count,reply_status,count);
    }
}


int main(int argc,char **argv){
    char *buff;
    int fd, sent_bytes, reply_status,buffer_status,reply_count, scan_status;
    unsigned int count;
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

    buff = malloc(BUFFER_SIZE* sizeof(char));
    clock = CLOCK_PROCESS_CPUTIME_ID;
    count = 1;

    while(1){
        buffer_status = sprintf(buff,"%d",count);
        if(buffer_status<0){
            perror("Cannot convert integer to char buffer");
            return 1;
        }
        sent_bytes = send_packet(fd,to,ip,buff);
        start_time = get_current_secs(clock);

        reply_status = handle_reply_with_timeout(fd,sent_bytes,buff);

        scan_status = sscanf(buff,"%d",&reply_count);
        if(scan_status<1){
            perror("Couldn't parse int from received bytes");
            exit(1);
        }
        end_time = get_current_secs(clock);

        print_status_with_counter(reply_status,reply_count,count,start_time,end_time);
        count++;
        sleep(1);
    }
}