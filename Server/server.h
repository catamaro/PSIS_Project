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
#define MAX_PLAYERS_WAITING 5

void * threadAccept(void *arg);
void * threadClient(void *arg);
int ** loadBoard(char* arg);
void bounceBounds(int x1, int y1, int* x_new, int* y_new);
void RandomPositionConnect(int* x1, int* x2, int* y1, int* y2);
void RandomPositionRules(int* x, int* y);
void checkRulesMonster(struct player* dealer, struct player* receiver, int x_new, int y_new,
						int* x_new1, int* y_new1, int* x_new2, int* y_new2,
							int* x_old1, int* y_old1, int* x_old2, int* y_old2);
void checkRulesPacman(struct player* dealer, struct player* receiver, int x_new, int y_new,
						int* x_new1, int* y_new1, int* x_new2, int* y_new2,
							int* x_old1, int* y_old1, int* x_old2, int* y_old2);
void checkRulesSuperPacman(struct player* dealer, struct player* receiver, int x_new, int y_new,
						int* x_new1, int* y_new1, int* x_new2, int* y_new2,
							int* x_old1, int* y_old1, int* x_old2, int* y_old2);
void clientDisconnect(int *sock_fd);

void PrintPlayerScore();
void ResetScore();
void IncrementMovement();
#endif
