#ifndef MAIN_H
#define MAIN_H

#include "UI_library.h"
#include <pthread.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct exe4_message{
  int character; // 2 pacman 3 monster
  int x;
  int y;
} exe4_message;

void * threadReceive(void *arg);
void * threadSend(void *arg);

#endif