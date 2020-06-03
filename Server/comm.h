#ifndef COMM_H
#define COMM_H

#include "structs.h"

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

int send_board(int x, int y, int sock_fd);
int rcv_color(int sock_fd, color *new_color);
int rcv_event(int sock_fd, SDL_Event *new_event, int *type);
int send_position(struct position * pacman, struct position *monster, int sock_fd);


#endif