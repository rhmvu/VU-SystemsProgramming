//
// Created by ruben on 9-9-18.
//

#ifndef SYSTEMSPROGRAMMING_PINGCLIENT1_H
#define SYSTEMSPROGRAMMING_PINGCLIENT1_H

int setup_socket();
struct in_addr* get_ip(const char *name);
void handle_reply(int fd, int sent_length, char *buff);
int send_packet(int fd,struct sockaddr_in to, struct in_addr *ip,char *buff);
double get_current_secs(clockid_t clock);


#endif //SYSTEMSPROGRAMMING_PINGCLIENT1_H
