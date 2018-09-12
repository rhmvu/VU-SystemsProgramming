//
// Created by ruben on 12-9-18.
//

#ifndef SYSTEMSPROGRAMMING_PINGUTILS_H
#define SYSTEMSPROGRAMMING_PINGUTILS_H
/*#include <sys/types.h>
#include <sys/socket.h>*/
#include <netinet/in.h>
#include <arpa/inet.h>


extern const int DEFAULT_PORT;
extern const int NANO_OFFSET;

int setup_socket();
struct in_addr* get_ip(const char *name);
void handle_reply(int fd, int sent_length, char *buff);
int send_packet(int fd,struct sockaddr_in to, struct in_addr *ip,char *buff);
double get_current_secs(clockid_t clock);

#endif //SYSTEMSPROGRAMMING_PINGUTILS_H