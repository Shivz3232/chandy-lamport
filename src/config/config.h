#ifndef CONFIG_H
#define CONFIG_H

char* name;
char* network;
char* inputHostname;
int tokenDelay;
int markerDelay;
int snapshotDelay;
int snapshotId;
int snapshotEnabled;

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
