#include "queue.h"

#include <stdio.h>

#include "../logger/logger.h"

void initializeQueue(Queue* q, int size) {
  q->items = malloc(sizeof(int) * size);
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

int enqueue(Queue* q, int value) {
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

int peek(Queue* q)
{
  if (isEmpty(q)) {
    debug("Trying to peek into an empty queue");
    return -1;
  }
  
  return q->items[q->front + 1];
}
