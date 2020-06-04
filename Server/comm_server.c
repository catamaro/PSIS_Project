#include "structs.h"
#include "server.h"
#include "comm.h"
#include "list_handler.h"

int send_board_dim(int x, int y, int sock_fd){
    char message[50];
    int err;

	memset(message, 0, 50*sizeof(char)); 

    err = sprintf(message, "%d %d\n", x, y);
	if(err == -1){
		printf("error: cannot create message\n");
		return -1;
	}

   	err = write(sock_fd, message, strlen(message)); 
	if(err <= 0){
		perror("write: ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

	printf("\nsvr snd board size: %s\n", message);

    return 0;
}

int send_board_setup(int sock_fd){
	struct pos_list *head = getBrickList();
	struct pos_list *current = head;
	struct player *headPlayer = getPlayerList();
	struct player *currentPlayer = headPlayer;
    int err;

	for (current = head; current != NULL; current = current->next)
	{
		err = send_init_msg(sock_fd, current->character, current->x, current->y, NULL);
		if(err == -1) return -1;
	}

	head = getFruitList();
	current = head;

	for (current = head; current != NULL; current = current->next)
	{
		err = send_init_msg(sock_fd, current->character, current->x, current->y, NULL);
		if(err == -1) return -1;
	}
	
	// end of messages of type 1
	err = send_init_msg(sock_fd, -1, -1, -1, NULL);
	if(err == -1) return -1;

	for (currentPlayer = headPlayer; currentPlayer != NULL; currentPlayer = currentPlayer->next)
	{

		err = send_init_msg(sock_fd, MONSTER, currentPlayer->monster->x, currentPlayer->monster->y, currentPlayer->rgb);
		if(err == -1) return -1;

		if(currentPlayer->times == 0)
			err = send_init_msg(sock_fd, PACMAN, currentPlayer->pacman->x, currentPlayer->pacman->y, currentPlayer->rgb);
		else
			err = send_init_msg(sock_fd, SUPERPACMAN, currentPlayer->pacman->x, currentPlayer->pacman->y, currentPlayer->rgb);
		
		if(err == -1) return -1;
	}
	
	// end of messages of type 2
	err = send_init_msg(sock_fd, -1, -1, -1, NULL);
	if(err == -1) return -1;

	return 0;
}

int send_init_msg(int sock_fd, int type, int x, int y, struct color *rgb){
	int err;

	if (type == PACMAN || type == MONSTER || type == SUPERPACMAN){
		struct init_msg_2 *message = malloc(sizeof(struct init_msg_2));
		message->r = rgb->r;
		message->g = rgb->g;
		message->b = rgb->b;
		message->character = type;
		message->x = x;
		message->y = y;
		
		err = write(sock_fd, message, sizeof(*message)); 
		if(err <= 0){
			perror("write: ");
			close(sock_fd);
			exit(EXIT_FAILURE);
		}
		printf("\nsvr snd initial positions: %d %d %d %d %d\n", message->x, message->y, 
					message->r, message->g, message->b);
	}
	else{
		struct init_msg_1 *message = malloc(sizeof(struct init_msg_1));
		message->character = type;
		message->x = x;
		message->y = y;

		err = write(sock_fd, message, sizeof(*message)); 
		if(err <= 0){
			perror("write: ");
			close(sock_fd);
			exit(EXIT_FAILURE);
		}
		printf("\nsvr snd initial positions: %d %d\n", message->x, message->y);
	}

	
	return 0;
}

int send_position(struct position *pacman, struct position *monster, int sock_fd){
	char message[50];
    int err;

	memset(message, 0, 50*sizeof(char)); 

    err = sprintf(message, "%d %d %d %d\n", pacman->x, pacman->y, monster->x, monster->y);
	if(err == -1){
		printf("error: cannot create message\n");
		return -1;
	}

   	err = write(sock_fd, message, strlen(message)); 
	if(err <= 0){
		perror("write: ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

	printf("\nsvr snd initial positions: %s\n", message);

    return 0;
}

int rcv_color(int sock_fd, color *new_color){
    int err;

	err = recv(sock_fd, new_color , sizeof(*new_color), 0);	
	if(err <= 0){
		perror("receive ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	} 

	printf("\nsvr rcv color: %d %d %d\n", new_color->r, new_color->g, new_color->b);
	
	return 0;
	/*player_list = getPlayerList();
	if (player_list == NULL) return 0; // there are no players

	while(player_list != NULL && unique_color){
		if(player_list->rgb->r == new_color->r && player_list->rgb->g == new_color->g && 
			player_list->rgb->b == new_color->b) unique_color = false;
		player_list = player_list->next;
	}

	if(!unique_color) return -1;
	else return 0;*/
	
}

int rcv_event(int sock_fd, SDL_Event *new_event, int *type){
	int err;
	int new_x, new_y, dir;
	struct player *list;
	struct position *new_position = malloc(sizeof(struct position));
	struct init_msg_1 *message = malloc(sizeof(struct init_msg_1));

	// falta usar mutex para impedir que várias threads corram ao mesmo tempo

	err = recv(sock_fd, message , sizeof(*message), 0);
	if(err <= 0){
		perror("receive ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

	list = getPlayerList();
	if (list == NULL) return -1; // there are no players

	while(list != NULL){
		if(sock_fd == list->sock_fd) break;
		list = list->next;
	}

	if(message->character == PACMAN){
		*type = PACMAN;

		printf("svr rcv event: %d %d %d\n", message->character, message->x, message->y);

		// store new position in motion 
		new_position->x = message->x;
		new_position->y = message->y;
		// store previous position in user data
		new_event->user.data1 = list;
		new_event->user.data2 = new_position;
	}
	else if(message->character == MONSTER){
		*type = MONSTER;
		
		printf("svr rcv event: %d %d\n", message->character, message->x);

		new_x = list->monster->x;
		new_y = list->monster->y;
		dir = message->x;
		
		switch (dir)
		{
			case LEFT:  new_x--; break;
			case RIGHT: new_x++; break;
			case UP: 	new_y--; break;
			case DOWN: 	new_y++; break;
		}
		new_position->x = new_x;
		new_position->y = new_y;

		// store previous position in user data
		new_event->user.data1 = list;
		new_event->user.data2 = new_position;
	}

	return 0;
}

int broadcast_update(int x_new, int y_new, int x, int y, int character, struct color *rgb){
	struct player *head = getPlayerList();
	struct player *current = head;
	int err;

	for (current = head; current != NULL; current = current->next)
	{
		err = send_update(current->sock_fd, character, x, y, x_new, y_new, rgb);
		if(err == -1) return -1;
	}

	return 0;
}
	
int send_update(int sock_fd, int type, int x, int y, int new_x, int new_y, struct color *rgb){
	struct update_msg *message = malloc(sizeof(struct update_msg));
	int err;

	message->character = type;
	message->new_x = new_x;
	message->new_y = new_y;
	message->x = x;
	message->y = y;
	if(rgb != NULL){
		message->r = rgb->r;
		message->g = rgb->g;
		message->b = rgb->b;
	}
	else{
		message->r = -1;
		message->g = -1;
		message->b = -1;
	}
	

	err = write(sock_fd, message, sizeof(*message)); 
	if(err <= 0){
		perror("write: ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

	return 0;
}

