#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>

#include "config/config.h"
#include "peers/peers.h"
#include "utils/utils.h"
#include "logger/logger.h"

struct Peer** peers;
void* hear(void*); // Because there already exists a method in inbuilt library called listen
void freePeers(struct Peer**);

int main(int argc, char const* argv[]) {
  info("Process started\n");

  initializeEnvVariables();
  debug("Successfully initialized environment Variables.\n");
  debug("============================================\n\n\n\n");
  return 0;

  peers = malloc(maxPeers * sizeof(struct Peer*));

  debug("============================================\n");
  debug("Parsing hosts file.\n");
  parseHostsfile(peers);
  debug("Successfully parsed hosts file.\n");
  debug("============================================\n\n\n\n");

  if (numPeers == 0) {
    info("Hostsfile empty, no peers found.\n");
    return 0;
  }

  struct addrinfo hints, *addr_info;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, port, &hints, &addr_info) < 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  debug("============================================\n");
  debug("Creating self socket.\n");
  int socket_fd;
  if ((socket_fd = bindToBest(addr_info)) < 0) {
    perror("bindToBest");
    exit(2);
  }
  debug("Self socket created successfully\n");
  debug("============================================\n\n\n\n");

  // ----------------------------------------------
  // Start listening for peers in a separate thread
  // ----------------------------------------------
  debug("============================================\n");
  debug("Starting listener.\n");
  pthread_t listener_thread;
  pthread_create(&listener_thread, NULL, hear, &socket_fd);
  pthread_detach(listener_thread);
  info("Listener started\n");
  debug("============================================\n\n\n\n");

  // ------------------------
  // Begin sending heartbeats
  // ------------------------
  char message[4] = "PING";
  while (1) {
    for (int i = 0; i < numPeers; i++) {
      int num_bytes;
      if ((num_bytes = sendto(socket_fd, &message, strlen(message), 0, peers[i]->chosen_addr_info->ai_addr, peers[i]->chosen_addr_info->ai_addrlen)) < 0) {
          perror("sendto");
          exit(1);
      }
    }

    sleep(10);
  }

  debug("============================================\n");
  close(socket_fd);
  freeaddrinfo(addr_info);
  freePeers(peers);
  debug("Socket closed\n");
  debug("Processs finished\n");
  debug("============================================\n\n\n\n");

  return 0;
}

void* hear(void* input) {  
  int socket_fd = *(int*)input;
  
  struct sockaddr_in peer_addr;
  socklen_t peer_addr_len = sizeof(peer_addr);

  int ready = 0;
  int numbytes;
  char message[maxMessageSize];
  while (ready == 0) {
    memset(&message, 0, (size_t) maxMessageSize);
    
    if ((numbytes = recvfrom(socket_fd, message, maxMessageSize-1, 0, (struct sockaddr*)&peer_addr, &peer_addr_len)) == -1) {
      perror("recvfrom");
      exit(1);
    }

    markPeerAsConnected(peers, &peer_addr, &peer_addr_len);

    if (connectedPeers == numPeers) {
      ready = 1;
    }
  }

  info("READY");

  info("Listener finished.\n");

  return NULL;
}

void freePeers(struct Peer **peers) {
    if (!peers) return;
    for (int i = 0; i < maxPeers; i++) {
        if (peers[i]) {
            free(peers[i]->name);
            free(peers[i]);
        }
    }
    free(peers);
}
