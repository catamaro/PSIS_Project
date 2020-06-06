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
#include <signal.h>

#define LEFT 12
#define UP 10
#define DOWN 11
#define RIGHT 13

#define EMPTY 0
#define BRICK 1
#define PACMAN 2
#define MONSTER 3
#define LEMON 4
#define CHERRY 5
#define SUPERPACMAN 6
#define SCORE 7
#define MAX_PLAYERS_WAITING 5

void * threadAccept(void *arg);
void * threadClient(void *arg);
void *threadClientTime(void *arg);

void clientDisconnect(int sock_fd);
void PrintPlayerScore();

void ResetScore();
void freeBoard();
void closeFd();

#endif
