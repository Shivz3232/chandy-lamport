#ifndef CONFIG_H
#define CONFIG_H

extern char* cEnv;
extern int processId;
extern char* hostName;
extern char* port;
extern int backlog;
extern int maxRetries;
extern int backoffDuration;
extern char* hostsFilePath;
extern int maxPeerNameSize;
extern int maxPeers;
extern int maxMessageSize;

void* initializeEnvVariables();

#endif
