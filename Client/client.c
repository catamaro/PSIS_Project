#include "client.h"
#include "structs.h"
#include "comm.h"

int done = 0;
int board_load = 0;
char *IP;
int last_x = 0, last_y = 0;


int main(int argc, char* argv[]){

	SDL_Event event;
	int port;
	int dir = -1;
	int board_x, board_y;
	struct sockaddr_in server_addr;
	struct player *my_player = malloc(sizeof(struct player));
	my_player->monster = malloc(sizeof(struct position));
	my_player->pacman = malloc(sizeof(struct position));
	my_player->rgb = malloc(sizeof(struct color));

	if(argc != 6){
		printf("error: invalid parameters [IP] [Port] [r] [g] [b]\n");
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
		if(my_player->rgb->r > 250 || my_player->rgb->r < 0){
			printf("error: invalid RGB color please use value from 0 to 250\n");
			exit(EXIT_FAILURE);
		}
		if(sscanf(argv[4], "%d", &(my_player->rgb->g)) != 1){
			printf("error: argv[4] is not a number\n");
			exit(EXIT_FAILURE);
		}
		if(my_player->rgb->g > 250 || my_player->rgb->g < 0){
			printf("error: invalid RGB color please use value from 0 to 250\n");
			exit(EXIT_FAILURE);
		}
		if(sscanf(argv[5], "%d", &(my_player->rgb->b)) != 1){
			printf("error: argv[5] is not a number\n");
			exit(EXIT_FAILURE);
		}
		if(my_player->rgb->b > 250 || my_player->rgb->b < 0){
			printf("error: invalid RGB color please use value from 0 to 250\n");
			exit(EXIT_FAILURE);
		}
	}


	/****** BEGIN OF SETUP ******/

	// setup of server - creation of client socket
	my_player->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(my_player->sock_fd == -1){
		perror("socket ");
		exit(EXIT_FAILURE);
	}
		
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	int err = inet_aton(IP, &server_addr.sin_addr);
	if(err == 0){
		perror("aton ");
		exit(EXIT_FAILURE);
	}
	// setup of server - creation of server connection to server
	err = connect(my_player->sock_fd, (const struct sockaddr *)&server_addr,
						sizeof(server_addr));

	if(err == -1){
		perror("conection ");
		exit(EXIT_FAILURE);
	}
	printf("just connected to the server \n\n");

	
	// client initial messages
	err = send_color(my_player->sock_fd, my_player->rgb);
	if(err == -1) exit(EXIT_FAILURE);

	err = rcv_board_dim(my_player->sock_fd, &board_x, &board_y);
	if (err == -1) exit(EXIT_FAILURE);
	 
	create_board_window(board_x, board_y);

	//receives messages from server
	pthread_create(&(my_player->thread_id), NULL, threadReceive, (void *)my_player);

	/****** END OF SETUP ******/


	// main thread send messages to server 
	int x = 0, y = 0;
	while(!done){
		while (SDL_PollEvent(&event)) {
			if(board_load != 2) continue;
			if(event.type == SDL_QUIT) {
				done = SDL_TRUE;
			}
			//when the mouse mooves the monster also moves
			if(event.type == SDL_MOUSEMOTION){
				get_board_place(event.motion.y, event.motion.x, &y, &x);

				if(x == last_x && y == last_y) continue;
				if( ( abs(y - my_player->pacman->y) + abs(x - my_player->pacman->x) ) == 1){

					err = send_event(PACMAN,  x,  y,  -1, my_player);
					if(err == -1) exit(EXIT_FAILURE);
					last_x = x; 
					last_y = y;
				}
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
	free(IP);

	close_board_windows();
	return EXIT_SUCCESS;
}

// thread that receives messages from server
void * threadReceive(void *arg){
	int err;
	struct player *my_player = (player*) arg;
	struct init_msg_1 *message1 = malloc(sizeof(struct init_msg_1));
	struct init_msg_2 *message2 = malloc(sizeof(struct init_msg_2));
	struct update_msg *message = malloc(sizeof(struct update_msg));
	struct color *rgb = malloc(sizeof(color));
	int new_x = 0, new_y = 0, character = 0;

	while(!done){
		// receive board info type 1 fruits and bricks 
		if(board_load == 0){
			err = recv(my_player->sock_fd, message1 , sizeof(*message1), 0);
			if(err <= 0){
				printf("Server has ended connection\n");
				serverClosed(my_player);
				free(message1);
				free(message2);
				free(message);
				free(rgb);
				exit(EXIT_FAILURE);
			}

			if(message1->character == -1){
				board_load++;
				free(message1);
				continue;
			}

			character = message1->character;
			new_x = message1->new_x;
			new_y = message1->new_y;
		}
		// receive board info type 1 pacman and monster
		else if(board_load == 1){
			err = recv(my_player->sock_fd, message2 , sizeof(*message2), 0);
			if(err <= 0){
				printf("Server has ended connection\n");
				serverClosed(my_player);
				free(message2);
				free(message);
				free(rgb);
				exit(EXIT_FAILURE);
			}
			
			if(message2->character == -1){
				board_load++;
				printf("board load completed\n");
				free(message2);
				continue;
			}

			character = message2->character;
			new_x = message2->new_x;
			new_y = message2->new_y;
			rgb->r = message2->r;
			rgb->g = message2->g;
			rgb->b = message2->b;
		}
		// receive board position update
		else if(board_load == 2){
			err = recv(my_player->sock_fd, message, sizeof(*message), 0);
			if(err == 0){
				printf("server has ended connection\n");
				serverClosed(my_player);
				free(message);
				free(rgb);
				exit(EXIT_FAILURE);
			}
			if(err == -1){
				perror("receive ");
				exit(EXIT_FAILURE);
			}

			if(message->character == SCORE){
				printf("Player %d: %d points\n",message->new_x,message->new_y);
				continue;
			} 

			if(message->x != -1)
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
				if(rgb->r == my_player->rgb->r && rgb->g == my_player->rgb->g && rgb->b == my_player->rgb->b){
					my_player->monster->x = new_x;
					my_player->monster->y = new_y;
				}
				break;
			case PACMAN: 
				paint_pacman(new_x,new_y, rgb->r,rgb->g, rgb->b); 
				if(rgb->r == my_player->rgb->r && rgb->g == my_player->rgb->g && rgb->b == my_player->rgb->b){
					my_player->pacman->x = new_x;
					my_player->pacman->y = new_y;
				}
				break;	
			case SUPERPACMAN: 
				paint_powerpacman(new_x,new_y, rgb->r,rgb->g, rgb->b); 
				if(rgb->r == my_player->rgb->r && rgb->g == my_player->rgb->g && rgb->b == my_player->rgb->b){
					my_player->pacman->x = new_x;
					my_player->pacman->y = new_y;
				}
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
	}

	free(message);
	free(rgb);

	return (NULL);
}

void serverClosed(struct player *my_player){
	close(my_player->sock_fd);
	free(my_player->monster);
	free(my_player->pacman);
	free(my_player->rgb);
	free(my_player);
	free(IP);
}
