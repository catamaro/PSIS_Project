#include "structs.h"
#include "client.h"
#include "comm.h"

int rcv_board_dim(int sock_fd, int *board_x, int *board_y){
    int err;
	struct position *board_dim = malloc(sizeof(struct position));

	err = recv(sock_fd, board_dim, sizeof(*board_dim), 0);
	if(err <= 0){
		perror("receive ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

    printf("\nclt rcv board size: %d %d\n", board_dim->x, board_dim->y);

	*board_x = board_dim->x;
	*board_y = board_dim->y;

	return 0;

}

int send_color(int sock_fd, struct color *new_color){
    int err;

	// send player color to server
	err = write(sock_fd, new_color, sizeof(*new_color)); 
	if(err <= 0){
		perror("write: ");
		close(sock_fd);
		return -1;
	}
	printf("\nclt snd color: %d %d %d\n", new_color->r, new_color->g, new_color->b);

	return 0;
}

int send_event(int type, int new_x, int new_y, int dir, struct player *my_player){
	int err;

	struct init_msg_1 *new_event = malloc(sizeof(struct init_msg_1));

	if(type == PACMAN){
		 
		new_event->character = PACMAN;
		new_event->x = new_x;
		new_event->y = new_y;
		
		err = write(my_player->sock_fd, new_event, sizeof(*new_event)); 
		if(err <= 0){
			perror("write: ");
			close(my_player->sock_fd);
			return -1;
		} 
		printf("clt snd update_msg: %d %d %d\n", new_event->character, new_event->x, new_event->y);
	}
	
	else if(type == MONSTER){

		new_event->character = MONSTER;
		new_event->x = dir;
		new_event->y = -1;
		
		err = write(my_player->sock_fd, new_event, sizeof(*new_event)); 
		if(err <= 0){
			perror("write: ");
			close(my_player->sock_fd);
			return -1;
		} 
		printf("clt snd update_msg: %d %d\n", new_event->character, new_event->x);
	}
	return 0;
}
