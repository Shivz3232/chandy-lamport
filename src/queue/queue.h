#ifndef QUEUE_H
#define QUEUE_H

typedef struct {
  int front;
  int rear;
  char** items;
} Queue;

void initializeQueue(Queue*);
int isEmpty(Queue*);
int isFull(Queue*);
int enqueue(Queue*, char*);
int dequeue(Queue*);
char* peek(Queue*);

#endif
