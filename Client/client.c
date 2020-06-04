#include "client.h"
#include "structs.h"
#include "comm.h"

int done = 0;

int main(int argc, char* argv[]){

	SDL_Event event;
	char *IP;
	int port;
	int dir = -1;
	int board_x, board_y;
	struct sockaddr_in server_addr;
	struct player *my_player = malloc(sizeof(struct player));
	my_player->monster = malloc(sizeof(struct position));
	my_player->pacman = malloc(sizeof(struct position));
	my_player->rgb = malloc(sizeof(struct color));

	if(argc != 6){
		printf("error");
		exit(EXIT_FAILURE);
	}
	else{
		IP = (char*) malloc(strlen(argv[1])+1);
		strcpy(IP, argv[1]);
		if(sscanf(argv[2], "%d", &port) != 1){
			printf("error: argv[2] is not a number\n");
			exit(EXIT_FAILURE);
		}
		if(sscanf(argv[3], "%d", &(my_player->rgb->r)) != 1){
			printf("error: argv[3] is not a number\n");
			exit(EXIT_FAILURE);
		}
		if(sscanf(argv[4], "%d", &(my_player->rgb->g)) != 1){
			printf("error: argv[4] is not a number\n");
			exit(EXIT_FAILURE);
		}
		if(sscanf(argv[5], "%d", &(my_player->rgb->b)) != 1){
			printf("error: argv[5] is not a number\n");
			exit(EXIT_FAILURE);
		}
	}

	my_player->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(my_player->sock_fd == -1){
		perror("socket: ");
		exit(EXIT_FAILURE);
	}
		
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	int err = inet_aton(IP, &server_addr.sin_addr);
	if(err == 0){
		perror("aton: ");
		exit(EXIT_FAILURE);
	}

	err = connect(my_player->sock_fd, (const struct sockaddr *)&server_addr,
						sizeof(server_addr));

	if(err == -1){
		perror("bind: ");
		exit(EXIT_FAILURE);
	}
	printf("just connected to the server \n\n");
	
	//receive and send thread id
	pthread_t receive_id;
	
	err = send_color(my_player->sock_fd, my_player->rgb);
	if(err == -1) exit(EXIT_FAILURE);

	err = rcv_board_dim(my_player->sock_fd, &board_x, &board_y);
	if (err == -1) exit(EXIT_FAILURE);
	printf("board x: %d board y: %d\n", board_x, board_y);
	 
	create_board_window(board_x, board_y);

	//receives messages from server
	pthread_create(&receive_id, NULL, threadReceive, (void *)my_player);
	
	while(!done){
		while (SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				done = SDL_TRUE;
			}
			//when the mouse mooves the monster also moves
			if(event.type == SDL_MOUSEMOTION){
				int x = 0, y = 0;

				get_board_place(event.motion.x, event.motion.y, &x, &y);

				if(x == my_player->pacman->x && y == my_player->pacman->y) continue;

				err = send_event(PACMAN,  x,  y,  -1, my_player);
				if(err == -1) exit(EXIT_FAILURE);
				
			}
			if(event.type == SDL_KEYDOWN){

				switch (event.key.keysym.sym)
				{
					case SDLK_LEFT:  dir = LEFT; break;
					case SDLK_RIGHT: dir = RIGHT; break;
					case SDLK_UP:    dir = UP; break;
					case SDLK_DOWN:  dir = DOWN; break;
				}

				err = send_event(MONSTER,  -1,  -1, dir, my_player);
				if(err == -1) exit(EXIT_FAILURE);
			}
		}
	}
	
	printf("fim\n");

	close(my_player->sock_fd);
	free(my_player->monster);
	free(my_player->pacman);
	free(my_player->rgb);
	free(my_player);

	close_board_windows();
	return EXIT_SUCCESS;
}

void * threadReceive(void *arg){
	int err, count = 0;
	player *my_player = (player*) arg;
	struct init_msg_1 *message1 = malloc(sizeof(struct init_msg_1));
	struct init_msg_2 *message2 = malloc(sizeof(struct init_msg_2));
	struct update_msg *message = malloc(sizeof(update_msg));
	struct color *rgb = malloc(sizeof(color));
	int new_x, new_y, character;

	while(!done){
		// receive board info type 1 fruits and bricks 
		if(count == 0){
			err = recv(my_player->sock_fd, message1 , sizeof(*message1), 0);
			if(err <= 0){
				perror("receive ");
				close(my_player->sock_fd);
				exit(EXIT_FAILURE);
			}
			if(message1->character == -1){
				count++;
				free(message1);
				continue;
			}
			character = message1->character;
			new_x = message1->x;
			new_y = message1->y;
		}
		// receive board info type 1 pacman and monster
		else if(count == 1){
			err = recv(my_player->sock_fd, message2 , sizeof(*message2), 0);
			if(err <= 0){
				perror("receive ");
				close(my_player->sock_fd);
				exit(EXIT_FAILURE);
			}
			if(message2->character == -1){
				count++;
				printf("board load completed\n");
				free(message2);
				continue;
			}
			character = message2->character;
			new_x = message2->x;
			new_y = message2->y;
			rgb->r = message2->r;
			rgb->g = message2->g;
			rgb->b = message2->b;
		}
		// receive board position update
		else if(count == 2){
			err = recv(my_player->sock_fd, message, sizeof(*message), 0);
			if(err <= 0){
				perror("receive ");
				close(my_player->sock_fd);
				exit(EXIT_FAILURE);
			}
			//validate_msg();

			clear_place(message->x, message->y);
			character = message->character;
			new_x = message->new_x;
			new_y = message->new_y;
			rgb->r = message->r;
			rgb->g = message->g;
			rgb->b = message->b;
		}
			
		switch (character)
		{
			case MONSTER:  
				paint_monster(new_x,new_y, rgb->r,rgb->g, rgb->b); 
				my_player->monster->x = new_x;
				my_player->monster->y = new_y;
				break;
			case PACMAN: 
				paint_pacman(new_x,new_y, rgb->r,rgb->g, rgb->b); 
				my_player->pacman->x = new_x;
				my_player->pacman->y = new_y;
				break;	
			case SUPERPACMAN: 
				paint_powerpacman(new_x,new_y, rgb->r,rgb->g, rgb->b); 
				my_player->pacman->x = new_x;
				my_player->pacman->y = new_y;
				break;
			case LEMON:    
				paint_lemon(new_x,new_y); 
				break;
			case CHERRY: 
				paint_cherry(new_x,new_y); 
				break;
			case BRICK: 
				paint_brick(new_x,new_y); 
				break;
		}
				
    	printf("received %d byte %d %d %d\n", err,character,new_x,new_y);
	}

	free(message);
	free(rgb);

	return (NULL);
}