#ifndef MAIN_H
#define MAIN_H

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

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

void * threadReceive(void *arg);
void * threadSend(void *arg);

#endif