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

int send_board_dim(int x, int y, int sock_fd);
int rcv_color(int sock_fd, color *new_color);
int rcv_event(int sock_fd, SDL_Event *new_event, int *type);
int send_update(int sock_fd, int type, int x, int y, int new_x, int new_y, struct color *rgb);
int send_board_setup(struct player *new_player);
int broadcast_update(int x_new, int y_new, int x, int y, int character, struct color *new_color);
int send_init_msg(int sock_fd, int type, int x, int y, struct color *rgb);
int broadcast_score(int player_id, int score);
int send_score(int sock_fd, int player_id, int score);
void accept_client(int board_x, int board_y, struct position *pacman, struct position *monster, 
					struct color *new_color, int *num_players, int new_fd);
int **CheckInactivity(int **board, struct player *my_player);
void clientDisconnect(int id, int *num_players, int ***board);

void sigHandler(int sig);
void init_insert_player_mutex();
void destroy_insert_player_mutex();
void init_run_rcv_event();
void destroy_run_rcv_event();
void init_run_snd_event();
void destroy_run_snd_event();

#endif