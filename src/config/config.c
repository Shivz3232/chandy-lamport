#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../logger/logger.h"

// CLI Arguments
char* name;
char* network;
char* inputHostname;
int tokenDelay = -1;
int markerDelay = -1;
int snapshotDelay = -1;
int snapshotId = -1;
int snapshotEnabled = 0;

// DEFAULTS
// char* cEnv = "development"; // To enable debug logs
char* cEnv = "production";
int processId = -1;
char* hostName;
char* port = "3000";
int backlog = 10;
int channelSize = 10;
int maxRetries = 5;
int backoffDuration = 5;
char* hostsFilePath = "hostsfile.txt";
int maxPeerNameSize = 100;
int maxPeers = 100;
int maxMessageSize = 100;

void* initializeCEnv();
void* initializeHostName();
void* initializePort();
void* initializeBacklog();
void* initializeChannelSize();
void* initializeMaxRetries();
void* initializeBackoffDuration();
void* initializeHostsFilePath();
void* initializeMaxPeerNameSize();
void* initializeMaxPeers();
void* initializeMaxMessageSize();

void* initializeEnvVariables() {
  initializeCEnv();
  initializePort();
  initializeBacklog();
  initializeChannelSize();
  initializeMaxRetries();
  initializeBackoffDuration();
  // initializeHostsFilePath();
  initializeMaxPeerNameSize();
  initializeHostName();
  initializeMaxPeers();
  initializeMaxMessageSize();

  return NULL;
}

void* initializeCEnv() {
  char* value = getenv("C_ENV");

  if (!value) {
    debug("C_ENV not found, defaulting to: %s", cEnv);
  } else {
    cEnv = strdup(value);
    debug("C_ENV set to %s", cEnv);
  }
  
  return NULL;
}

void* initializeHostName() {
  char value[maxPeerNameSize];

  if (gethostname(value, sizeof(value)) != 0) {
    perror("gethostname");
    exit(1);
  }

  hostName = strdup(value);
  
  debug("HOSTNAME set to %s", hostName);

  return NULL;
}

void* initializePort() {
  char* value = getenv("PORT");

  if (!value) {
    debug("PORT not found, defaulting to: %s", port);
  } else {
    port = strdup(value);
    debug("PORT set to %s", port);
  }

  return NULL;
}

void* initializeBacklog() {
  char* value = getenv("BACKLOG");

  if (!value) {
    debug("BACKLOG not found, defaulting to: %d", backlog);
  } else {
    backlog = atoi(value);
    debug("BACKLOG set to %d", backlog);
  }

  return NULL;
}

void* initializeChannelSize() {
  char* value = getenv("CHANNEL_SIZE");

  if (!value) {
    debug("CHANNEL_SIZE not found, defaulting to: %d", channelSize);
  } else {
    channelSize = atoi(value);
    debug("CHANNEL_SIZE set to %d", channelSize);
  }

  return NULL;
}

void* initializeMaxRetries() {
  char* value = getenv("MAX_RETRIES");

  if (!value) {
    debug("MAX_RETRIES not found, defaulting to: %d", maxRetries);
  } else {
    maxRetries = atoi(value);
    debug("MAX_RETRIES set to %d", maxRetries);
  }

  return NULL;
}

void* initializeBackoffDuration() {
  char* value = getenv("backoffDuration");

  if (!value) {
    debug("backoffDuration not found, defaulting to: %d", backoffDuration);
  } else {
    backoffDuration = atoi(value);
    debug("backoffDuration set to %d", backlog);
  }

  return NULL;
}

void* initializeHostsFilePath() {
  char* value = getenv("HOSTSFILE_PATH");

  if (!value) {
    debug("HOSTSFILE_PATH not found, defaulting to: %s", hostsFilePath);
  } else {
    hostsFilePath = strdup(value);
    debug("HOSTSFILE_PATH set to %s", hostsFilePath);
  }

  return NULL;
}

void* initializeMaxPeerNameSize() {
  char* value = getenv("MAX_PEER_NAME_SIZE");

  if (!value) {
    debug("MAX_PEER_NAME_SIZE not found, defaulting to: %d", maxPeerNameSize);
  } else {
    maxPeerNameSize = atoi(value);
    debug("MAX_PEER_NAME_SIZE set to %d", maxPeerNameSize);
  }
  
  return NULL;
}

void* initializeMaxPeers() {
  char* value = getenv("MAX_PEERS");

  if (!value) {
    debug("MAX_PEERS not found, defaulting to: %d", maxPeers);
  } else {
    maxPeers = atoi(value);
    debug("MAX_PEERS set to %d", maxPeers);
  }
  
  return NULL;
}

void* initializeMaxMessageSize() {
  char* value = getenv("MAX_MESSAGE_SIZE");

  if (!value) {
    debug("MAX_MESSAGE_SIZE not found, defaulting to: %d", maxMessageSize);
  } else {
    maxMessageSize = atoi(value);
    debug("MAX_MESSAGE_SIZE set to %d", maxMessageSize);
  }
  
  return NULL;
}
