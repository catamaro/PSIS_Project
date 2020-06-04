#include "structs.h"
#include "comm.h"

int rcv_board(int sock_fd, int *board_x, int *board_y){
    int x, y, err;
	char message[50];

	memset(message, 0, 50*sizeof(char)); 

	err = recv(sock_fd, &message , sizeof(message), 0);
	if(err <= 0){
		perror("receive ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

    printf("\nclt rcv board size: %s\n", message);
    if(sscanf(message, "%d %d", &x, &y) == 2){
		*board_x = x;
		*board_y = y;
		return 0;
    }
    else{
        printf("error: incorrect initial message");
        return -1;
    }
}

int rcv_position(struct player *my_player){
	int x1, y1, x2, y2, err;
	char message[50];

	memset(message, 0, 50*sizeof(char)); 

	err = recv(my_player->sock_fd, &message , sizeof(message), 0);
	if(err <= 0){
		perror("receive ");
		close(my_player->sock_fd);
		exit(EXIT_FAILURE);
	}

    printf("\nclt rcv initial position %s\n", message);
    if(sscanf(message, "%d %d %d %d", &x1, &y1, &x2, &y2) == 4){
		my_player->pacman->x = x1;
		my_player->pacman->y = y1;
		my_player->monster->x = x2;
		my_player->monster->y = y2;

		paint_pacman(my_player->pacman->x, my_player->pacman->y , my_player->p_color->r, 
						my_player->p_color->g, my_player->p_color->b);
		paint_monster(my_player->monster->x, my_player->monster->y , my_player->p_color->r, 
						my_player->p_color->g, my_player->p_color->b);

		return 0;
    }
    else{
        printf("error: incorrect initial message\n");
        return -1;
    }
}

int send_color(int sock_fd, struct color *new_color){
    int err;

	// send player color to server
	err = write(sock_fd, new_color, sizeof(*new_color)); 
	if(err == -1){
		perror("write: ");
		return -1;
	}
	printf("\nclt snd color: %d %d %d\n", new_color->r, new_color->g, new_color->b);

	return 0;

	// receives confirmation of valid color
	/*err = recv(sock_fd, message , sizeof(message), 0);	
	if (err == -1){
		perror("receive: ");
		exit(EXIT_FAILURE);
	} 

	printf("clt rcv color confirm: %s\n", message);

	if(sscanf(message, "%d", &valid) == 1){
		if(valid == 0){
			printf("error: color already in use\n");
			return -1;
		}
		else if(valid == 1) printf("color is avaiable\n");
		else return -1;
    }
    else{
        printf("error: incorrect initial message");
        return -1;
    }*/
}

int send_event(int type, int new_x, int new_y, int direction, struct player *my_player){
	char message[50];
	int err;

	memset(message, 0, 50*sizeof(char)); 

	if(type == PACMAN){
		 
		err = sprintf(message, "%d %d %d\n", PACMAN, new_x, new_y);
		if(err == -1){
			printf("error: cannot create message\n");
			return -1;
		}
		
		err = write(my_player->sock_fd, message, strlen(message)); 
		if(err == -1){
			perror("write: ");
			return -1;
		} 
		printf("clt snd pos_update: %s %d\n", message, PACMAN);
		fflush(stdout);
	}
	else if(type == MONSTER){
		err = sprintf(message, "%d %d\n", MONSTER, direction);
		if(err == -1){
			printf("error: cannot create message\n");
			return -1;
		}
		
		err = write(my_player->sock_fd, message, strlen(message)); 
		if(err == -1){
			perror("write: ");
			return -1;
		} 
		printf("clt snd pos_update: %s %d\n", message, MONSTER);
		fflush(stdout);
	}
	return 0;
}
