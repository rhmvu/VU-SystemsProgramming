#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX_BUFFER_SIZE 1024
#define STDOUT_FD 1

typedef struct node{
    char buffer[MAX_BUFFER_SIZE];
    int bytes_read;
    struct node* previous;
} node_t;


node_t* read_bytes(int fd,node_t* previous_node){
    node_t *current_node;
    do {
        //create new node
        current_node = malloc(sizeof(node_t));
        if (current_node == NULL) {
            perror("Out of memory!");
            exit(1);
        }
        current_node->previous = previous_node;
        //read bytes to the buffer of the current node and store the amount of bytes read
        current_node->bytes_read = read(fd, current_node->buffer, MAX_BUFFER_SIZE);
        previous_node = current_node;
    }while (current_node->bytes_read == MAX_BUFFER_SIZE);
    if(current_node->bytes_read<0){
        perror("Input File READ error");
        exit(1);
    }
    return current_node;
}

void print_bytes_reversed(node_t* current_node){
    int i;
    node_t *previous_node;
    char reversed[MAX_BUFFER_SIZE];

    do {
        //reverse the byte buffer in the node
        for (i = 0; i < current_node->bytes_read; ++i) {
            reversed[i] = current_node->buffer[current_node->bytes_read - 1 - i];
        }
        //print it to stdout
        if (write(STDOUT_FD, reversed, current_node->bytes_read) != current_node->bytes_read) {
            perror("Error printing reversed bytes");
            exit(1);
        }
        previous_node = current_node->previous;
        free(current_node);
        current_node = previous_node;
    }while (previous_node !=NULL);
}

int main(int argc, char **argv) {
    int input_filedesc,input_file_close_status;
    node_t *tail;

    if (argc != 2) {
        printf("Usage: reverse <input filepath>\n\n");
        return 1;
    }

    input_filedesc = open(argv[1], O_RDONLY);
    if (input_filedesc < 0){
        perror("Input File ERROR:");
        return 1;
    }

    tail = read_bytes(input_filedesc, NULL);

    input_file_close_status = close(input_filedesc);
    if(input_file_close_status < 0){
        perror("Input File ERROR:");
        return 1;
    }
    print_bytes_reversed(tail);
    return 0;
}