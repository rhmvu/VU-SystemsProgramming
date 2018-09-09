//
// Created by ruben on 9-9-18.
//

#ifndef SYSTEMSPROGRAMMING_PINGCLIENT2_H
#define SYSTEMSPROGRAMMING_PINGCLIENT2_H

int handle_reply_with_timeout(int fd, int sent_length, char *buff);
void print_status(int reply_status,double start_time, double end_time);


#endif //SYSTEMSPROGRAMMING_PINGCLIENT2_H
