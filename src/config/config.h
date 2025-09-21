#ifndef CONFIG_H
#define CONFIG_H

extern char* cEnv;
extern char* hostName;
extern char* port;
extern char* hostsFilePath;
extern int maxPeerNameSize;
extern int maxPeers;
extern int maxMessageSize;

void* initializeEnvVariables();

#endif
