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

int rcv_board(char *message, int *board_x, int *board_y);
int send_color(int server_fd, struct color *new_color);

#endif