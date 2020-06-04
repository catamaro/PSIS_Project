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

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

#define EMPTY 0
#define BRICK 1
#define PACMAN 2
#define MONSTER 3
#define LEMON 4
#define CHERRY 5
#define SUPERPACMAN 6
#define FRUIT 7
#define MAX_PLAYERS_WAITING 5

void * threadAccept(void *arg);
void * threadClient(void *arg);
int ** loadBoard(char* arg);
void bounceBounds(int x1, int y1, int* x_new, int* y_new);
void RandomPositionConnect(int* x1, int* x2, int* y1, int* y2);
void RandomPositionRules(int* x, int* y);
void checkRulesMonster(struct player *dealer, struct player *receiver, int *x_new, 
						int *y_new, int *x_new_2, int *y_new_2);
void checkRulesPacman(struct player* dealer, struct player* receiver,
                        int *x_new, int *y_new, int *x_new_2, int *y_new_2);
void checkRulesSuperPacman(struct player *dealer, struct player *receiver, int *x_new, 
						int *y_new, int *x_new_2, int *y_new_2);
void ManageFruits();

#endif
