#include "token.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../config/config.h"
#include "../logger/logger.h"
#include "../queue/queue.h"
#include "../peers/peers.h"

void* passToken(void* input) {  
  struct Peer** peers = input;
  if (input == NULL) {
    debug("passToken: input NULL!!\n");
    return NULL;
  }

  struct Peer* predecessor = getPredecessor(peers);
  if (input == NULL) {
    debug("passToken: predecessor NULL!!\n");
    return NULL;
  }

  if (isEmpty(predecessor->read_channel)) {
    debug("passToken: Nothing to pass!!");
    return NULL;
  }

  char* token = peek(predecessor->read_channel);
  if (token == NULL) {
    debug("peek: Token not found!!\n");
    return NULL;
  }

  int dequeued = dequeue(predecessor->read_channel);
  if (dequeued != 1) {
    debug("dequeue: Failed to dequeue token!!\n");
    return NULL;
  }

  struct Peer* successor = getSuccessor(peers);

  debug("Sleeping for %d seconds before forwarding the token\n", tokenDelay);
  sleep(tokenDelay);
  debug("Woke up, sending token\n");
  
  int numBytesSent;
  if ((numBytesSent = send(successor->write_socket_fd, token, strlen(token), 0)) < 0) {
    debug("send: Failed. numBytesSent: %d\n", numBytesSent);
    return NULL;
  }

  if (numBytesSent == 0) {
    debug("Nothing was sent. numBytesSent is 0!!\n");
  } else if (numBytesSent < strlen(token)) {
    debug("Partial token was sent!!\n");
  } else {
    debug("Token forwarded successfully\n");
  }

  free(token);
  
  return NULL;
}

void* startTokenPassing(void* input) {
  struct Peer** peers = input;
  if (input == NULL) {
    debug("passToken: input NULL!!\n");
    return NULL;
  }

  struct Peer* successor = getSuccessor(peers);
  if (input == NULL) {
    debug("passToken: predecessor NULL!!\n");
    return NULL;
  }

  debug("Sleeping for %d seconds before forwarding the token\n", tokenDelay);
  sleep(tokenDelay);
  debug("Woke up, sending token\n");

  char token[] = "token";
  int numBytesSent;
  if ((numBytesSent = send(successor->write_socket_fd, token, strlen(token), 0)) < 0) {
    perror("send");
    debug("send: Failed. numBytesSent: %d\n", numBytesSent);
    return NULL;
  }

  if (numBytesSent == 0) {
    debug("Nothing was sent. numBytesSent is 0!!\n");
  } else if (numBytesSent < strlen(token)) {
    debug("Partial token was sent!!\n");
  } else {
    debug("Token forwarded successfully\n");
  }
  
  return NULL;
}


