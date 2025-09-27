#include <pthread.h>

#ifndef QUEUE_H
#define QUEUE_H

typedef struct {
  int front;
  int rear;
  char** items;
  pthread_mutex_t lock;
} Queue;

void initializeQueue(Queue*);
int isEmpty(Queue*);
int isFull(Queue*);
int enqueue(Queue*, char*);
int dequeue(Queue*);
char* peek(Queue*);

#endif
