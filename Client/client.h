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

#define LEFT 10
#define RIGHT 11
#define UP 12
#define DOWN 13

#define EMPTY 0
#define BRICK 1
#define PACMAN 2
#define MONSTER 3
#define LEMON 4
#define CHERRY 5
#define SUPERPACMAN 6
#define SCORE 7

void * threadReceive(void *arg);
void * threadSend(void *arg);

#endif