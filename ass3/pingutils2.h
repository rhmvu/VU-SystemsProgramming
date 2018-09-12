//
// Created by ruben on 12-9-18.
//

#ifndef SYSTEMSPROGRAMMING_PINGUTILS2_H
#define SYSTEMSPROGRAMMING_PINGUTILS2_H

int handle_reply_with_timeout(int fd, int sent_length, char *buff);
void print_status(int reply_status,double start_time, double end_time);


#endif //SYSTEMSPROGRAMMING_PINGUTILS2_H
