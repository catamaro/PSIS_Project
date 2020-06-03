#include "structs.h"
#include "server.h"
#include "comm.h"
#include "list_handler.h"

int send_board(int x, int y, int sock_fd){
    char message[50];
    int err;

	memset(message, 0, 50*sizeof(char)); 

    err = sprintf(message, "%d %d\n", x, y);
	if(err == -1){
		printf("error: cannot create message\n");
		return -1;
	}

   	err = write(sock_fd, message, strlen(message)); 
	if(err == -1){
		perror("write: ");
		return -1;
	}

	printf("\nsvr snd board size: %s\n", message);

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
	if(err == -1){
		perror("write: ");
		return -1;
	}

	printf("\nsvr snd initial positions: %s\n", message);

    return 0;
}

int rcv_color(int sock_fd, color *new_color){
    int err;
	bool unique_color = true;
	struct player *player_list;

	// falta usar mutex para impedir que várias threads corram ao mesmo tempo

	err = recv(sock_fd, new_color , sizeof(*new_color), 0);	
	if (err == -1){
		perror("receive: ");
		exit(EXIT_FAILURE);
	} 

	printf("\nsvr rcv color: %d %d %d\n", new_color->r, new_color->g, new_color->b);

	player_list = getPlayerList();
	if (player_list == NULL) return 0; // there are no players

	while(player_list != NULL && unique_color){
		if(player_list->p_color->r == new_color->r && player_list->p_color->g == new_color->g && 
			player_list->p_color->b == new_color->b) unique_color = false;
		player_list = player_list->next;
	}

	if(!unique_color) return -1;
	else return 0;


	/*if(!unique_color){
		err = sprintf(message, "%d\n", 0);
		if(err == -1){
			printf("error: cannot create message\n");
			return -1;
		}
		err = write(sock_fd, message, strlen(message)); 
		if(err == -1){
			perror("write: ");
			exit(EXIT_FAILURE);
		}
		//rcv_color(sock_fd, new_color);	
		return -1;
	}
	else{
		err = sprintf(message, "%d\n", 1);
		if(err == -1){
			printf("error: cannot create message\n");
			return -1;
		}
		err = write(sock_fd, message, strlen(message)); 
		if(err == -1){
			perror("write: ");
			exit(EXIT_FAILURE);
		}
	} */
}

int rcv_event(int sock_fd, SDL_Event *new_event, int *type){
	int err;
	char message[50];
	int new_x, new_y, dir;
	struct player *list;

	memset(message, 0, 50*sizeof(char)); 

	// falta usar mutex para impedir que várias threads corram ao mesmo tempo

	err = recv(sock_fd, message , sizeof(message), 0);
	if (err == -1){
		perror("receive: ");
		return -1;
	} 

	list = getPlayerList();
	if (list == NULL) return -1; // there are no players

	while(list != NULL){
		if(sock_fd == list->sock_fd) break;
		list = list->next;
	}

	if(sscanf(message, "%d", type) == 1){
		if(*type == PACMAN){
			if(sscanf(message, "%d %d %d", type, &new_x, &new_y) == 3){

				printf("svr rcv event: %s\n", message);

				struct position *new_position = malloc(sizeof(struct position));
				// store new position in motion 
				new_position->x = new_x;
				new_position->y = new_y;
				// store previous position in user data
				new_event->user.data1 = list;
				new_event->user.data2 = new_position;
			}
			else{
				printf("error: invalid message\n");
				return -1;
			}
		}
		else if(*type == MONSTER){
			if(sscanf(message, "%d %d", type, &dir) == 2){
				printf("\nsvr rcv event: %s\n", message);

				new_x = list->monster->x;
				new_y = list->monster->y;
				
				switch (dir)
				{
					case LEFT:  new_x--; break;
					case RIGHT: new_x++; break;
					case UP: 	new_y--; break;
					case DOWN: 	new_y++; break;
				}
				struct position *new_position = malloc(sizeof(struct position));
				new_position->x = new_x;
				new_position->y = new_y;

				// store previous position in user data
				new_event->user.data1 = list;
				new_event->user.data2 = new_position;
			}
			else{
				printf("error: invalid message\n");
				return -1;
			}
		}
		else{
			printf("error: invalid message\n");
			return -1;
		}
	}
	else{
		printf("error: invalid message\n");
		return -1;
	}
	

	return 0;
}


