#include "token.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../config/config.h"
#include "../logger/logger.h"
#include "../queue/queue.h"
#include "../peers/peers.h"

void* passToken(void* input) {
  if (input == NULL) {
    debug("passToken: input NULL!!\n");
    return NULL;
  }
  
  state += 1;
  info("{proc_id: %d, state: %d}", processId + 1, state);
  
  struct Peer** peers = input;

  struct Peer* predecessor = getPredecessor(peers);
  if (predecessor == NULL) {
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

  if (dequeue(predecessor->read_channel) < 0) {
    debug("dequeue: Failed to dequeue token!!\n");
    return NULL;
  }

  struct Peer* successor = getSuccessor(peers);
  if (successor == NULL) {
    debug("passToken: successor NULL!!\n");
    return NULL;
  }

  debug("Sleeping for %0.2f seconds before forwarding the token\n", tokenDelay);
  usleep(tokenDelay * 1000000);
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
    info("{proc_id: %d, sender: %d, receiver: %d, message:\"token\"}", processId + 1, predecessor->id + 1, successor->id + 1);
  }

  free(token);
  
  return NULL;
}

void* startTokenPassing(void* input) {
  state += 1;
  info("{proc_id: %d, state: %d}", processId + 1, state);
  
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

  debug("Sleeping for %0.2f seconds before forwarding the token\n", tokenDelay);
  usleep(tokenDelay * 1000000);
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
    info("{proc_id: %d, sender: %d, receiver: %d, message:\"token\"}", processId + 1, NULL, successor->id + 1);
  }
  
  return NULL;
}


