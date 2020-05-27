#ifndef SERVER_H
#define SERVER_H

#include "UI_library.h"
#include <pthread.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <unistd.h>

void * threadAccept(void *arg);
void * threadClient(void *arg);
int ** loadBoard(char* arg);

#endif