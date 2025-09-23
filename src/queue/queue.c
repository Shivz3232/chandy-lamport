#include "queue.h"

#include <stdlib.h>

#include "../config/config.h"
#include "../logger/logger.h"

void initializeQueue(Queue* q) {
  q->items = malloc(sizeof(char *) * channelSize);
  q->front = -1;
  q->rear = 0;
}

int isEmpty(Queue* q) {
  if (q->front == q->rear - 1) {
    return 1;
  } else {
    return 0;
  }
}

int isFull(Queue* q) {
  if (q->rear == channelSize) {
    return 1;
  } else {
    return 0;
  }
}

int enqueue(Queue* q, char* value) {
  if (isFull(q)) {
    debug("Trying to enqueue to a full queue\n");
    return -1;
  }
  
  q->items[q->rear] = value;
  q->rear++;

  return 1;
}

int dequeue(Queue* q) {
  if (isEmpty(q)) {
    debug("Trying to deque from an empty queue\n");
    return -1;
  }

  q->front++;

  return 1;
}

char* peek(Queue* q)
{
  if (isEmpty(q)) {
    debug("Trying to peek into an empty queue");
    return NULL;
  }
  
  return q->items[q->front + 1];
}
