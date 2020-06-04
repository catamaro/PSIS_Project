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

int rcv_board_dim(int sock_fd, int *board_x, int *board_y);
int send_color(int server_fd, struct color *new_color);
int send_event(int type, int new_x, int new_y, int dir, struct player *my_player);

#endif