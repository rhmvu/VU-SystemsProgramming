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


node_t* recursive_read_bytes(int fd,node_t* previous_buffer){
    //create new node
    node_t *current_buffer = malloc(sizeof(node_t));
    if (current_buffer == NULL) {
        perror("Out of memory!");
        exit(1);
    }
    current_buffer->previous = previous_buffer;
    //read bytes to the buffer of the current node and store the amount of bytes read
    current_buffer->bytes_read = read(fd,current_buffer->buffer,MAX_BUFFER_SIZE);
    if(current_buffer->bytes_read < MAX_BUFFER_SIZE){
        if(current_buffer->bytes_read<0){
            perror("Input File READ error");
            exit(1);
        }
        return current_buffer;
    }
    return recursive_read_bytes(fd,current_buffer);
}

void recursive_print_bytes_reversed(node_t* current_buffer){
    char reversed[current_buffer->bytes_read];
    int i;
    //reverse the byte buffer in the node
    for (i = 0; i < current_buffer->bytes_read; ++i) {
        reversed[i] = current_buffer->buffer[current_buffer->bytes_read-1-i];
    }
    //print it to stdout
    if(write(STDOUT_FD,reversed,current_buffer->bytes_read)!= current_buffer->bytes_read){
        perror("Error printing reversed bytes");
        exit(1);
    }
    node_t *previous_node = current_buffer->previous;
    free(current_buffer);
    if(previous_node!=NULL){
        recursive_print_bytes_reversed(previous_node);
    }
}

int main(int argc, char **argv) {
    int input_filedesc;
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

    tail = recursive_read_bytes(input_filedesc, NULL);

    if(close(input_filedesc) < 0){
        perror("Input File ERROR:");
        return 1;
    }
    recursive_print_bytes_reversed(tail);
    return 0;
}
