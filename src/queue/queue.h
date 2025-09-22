#ifndef QUEUE_H
#define QUEUE_H

typedef struct {
  int front;
  int rear;
  int closed;
  int* items;
} Queue;

void initializeQueue(Queue*, int);
int isEmpty(Queue*);
int isFull(Queue*);
int enqueue(Queue*, int);
int dequeue(Queue*);
int peek(Queue*);

#endif
