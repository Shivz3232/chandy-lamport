#ifndef QUEUE_H
#define QUEUE_H

typedef struct {
    int items[];
    int front;
    int rear;
    int closed;
} Queue;

void initializeQueue(Queue*, int);
bool isEmpty(Queue*);
bool isFull(Queue*);
int enqueue(Queue*, int);
int dequeue(Queue*);
int peek(Queue*);

#endif
