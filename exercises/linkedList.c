//
// Created by ruben on 5-9-18.
//

#include <stdio.h>
#include <stdlib.h>


struct node *head;
struct node *tail;

struct node{
    int value;
    struct node* next;
    struct node* previous;
};


void add_front(int value){
    struct node *newNode = malloc(sizeof(struct node));
    newNode->next = head;
    newNode->previous = NULL;
    head->previous = newNode;
    newNode->value = value;
    head = newNode;
}


void add_back(int value){
    struct node *newNode = malloc(sizeof(struct node));
    tail->next = newNode;
    newNode->previous = tail;
    newNode->next = NULL;
    newNode->value = value;
    tail = newNode;
}

void display_list(struct node *headNode){
    do{
        printf("%d  ",headNode->value);
        headNode = headNode->next;

    }while(headNode != NULL);
}

int main(int argc, char **argv){

    struct node *start = malloc(sizeof(struct node));

    start->previous = NULL;
    start->next = NULL;
    start->value = 5;

    head = start;
    tail = start;

    add_front(4);
    add_front(3);
    add_front(2);
    add_front(1);
    add_back(6);
    add_back(7);
    add_back(8);

    display_list(head);
    return 0;
}

