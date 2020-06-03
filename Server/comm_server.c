#include "structs.h"
#include "server.h"
#include "comm.h"
#include "list_handler.h"

int send_board(int x, int y, player *new_player){
    char message[50];
    int err;

    err = sprintf(message, "b %d %d\n", x, y);
	if(err == -1){
		printf("error: cannot create message\n");
		return -1;
	}
	//printf("message: %s\n", message);

   	err = write(new_player->sock_fd, message, strlen(message)); 
	if(err == -1){
		perror("write: ");
		return -1;
	}

    return 0;
}

int rcv_color(int sock_fd){
    int err;
	bool unique_color = true;
	struct player *player_list;
	struct color *new_color = malloc(sizeof(struct color));


	// falta usar mutex para impedir que várias threads corram ao mesmo tempo

    err = recv(sock_fd, new_color , sizeof(*new_color), 0);
	if (err == -1){
		perror("receive: ");
		exit(EXIT_FAILURE);
	} 

	player_list = getPlayerHead();
	if (player_list == NULL) return 0; // there are no players

	while(player_list != NULL && unique_color){
		if(player_list->p_color->r != new_color->r && player_list->p_color->g != new_color->g && 
			player_list->p_color->b != new_color->b) unique_color = false;
		player_list = player_list->next;
	}

	if(!unique_color) return -1;

	printf("r: %d g: %d b: %d\n", new_color->r, new_color->g, new_color->b);

	return 0;
}

int rcv_event(int sock_fd, SDL_Event *new_event){
	int err;
	char message[50];
	int new_x, new_y, dir;
	player *list;

	// falta usar mutex para impedir que várias threads corram ao mesmo tempo

	err = recv(sock_fd, message , sizeof(message), 0);
	if (err == -1){
		perror("receive: ");
		exit(EXIT_FAILURE);
	} 

	list = getPlayerHead();
	if (list == NULL) return -1; // there are no players

	while(list != NULL){
		if(sock_fd == list->sock_fd) break;
		list = list->next;
	}

	printf("message: %s\n", message);
    if(sscanf(message, "%d %d", &new_x, &new_y) == 2){
		*new_event.type = SDL_MOUSEMOTION;
		*new_event.motion.new_x = new_x;
		*new_event.motion.new_y = new_y;
		*new_event.motion.x = list->pacman->x;
		*new_event.motion.y = list->pacman->y;
	}
	else if(sscanf(message, "%d", &dir) == 1){

		new_event.type == SDL_KEYDOWN;
		switch (dir)
		{
			case LEFT:  
				*new_event.key.keysym.sym = SDLK_LEFT;
				*new_event.motion.new_x = list->pacman->x --; break;
			case RIGHT: 
				*new_event.key.keysym.sym = SDLK_RIGHT;
				*new_event.motion.new_x = list->pacman->x ++; break;
			case UP:    
				*new_event.key.keysym.sym = SDLK_UP;
				*new_event.motion.y = list->pacman->y --; break;
			case DOWN:  
				*new_event.key.keysym.sym = SDLK_DOWN;
				*new_event.motion.y = list->pacman->y ++; break;
		}

		*new_event.motion.x = list->pacman->x;
		*new_event.motion.y = list->pacman->y;
	}

	return 0;
}


