#include "queue.h"

#include <stdbool.h>
#include <stdio.h>

#include "../logger/logger.h"

void initializeQueue(Queue* q, int size) {
  q->items = malloc(sizeof(int) * size);
  q->front = -1;
  q->rear = 0;
}

bool isEmpty(Queue* q) { return (q->front == q->rear - 1); }

bool isFull(Queue* q) { return (q->rear == channelSize); }

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
