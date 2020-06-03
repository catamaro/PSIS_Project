#include "client.h"
#include "structs.h"
#include "comm.h"

int board_x;
int board_y;
int done = 0;

int main(int argc, char* argv[]){

	SDL_Event event;
	char *IP;
	int port;
	int dir = -1;
	struct sockaddr_in server_addr;
	struct player *my_player = malloc(sizeof(struct player));
	my_player->monster = malloc(sizeof(struct position));
	my_player->pacman = malloc(sizeof(struct position));
	my_player->p_color = malloc(sizeof(struct color));

	if(argc != 6){
		printf("error");
		exit(EXIT_FAILURE);
	}
	else{
		IP = (char*) malloc(strlen(argv[1]));
		strcpy(IP, argv[1]);
		if(sscanf(argv[2], "%d", &port) != 1){
			printf("error: argv[2] is not a number\n");
			exit(EXIT_FAILURE);
		}
		if(sscanf(argv[3], "%d", &(my_player->p_color->r)) != 1){
			printf("error: argv[3] is not a number\n");
			exit(EXIT_FAILURE);
		}
		if(sscanf(argv[4], "%d", &(my_player->p_color->g)) != 1){
			printf("error: argv[4] is not a number\n");
			exit(EXIT_FAILURE);
		}
		if(sscanf(argv[5], "%d", &(my_player->p_color->b)) != 1){
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

	//receives messages from server
	pthread_create(&receive_id, NULL, threadReceive, (void *)my_player);

	err = send_color(my_player->sock_fd, my_player->p_color);
	if(err == -1) exit(EXIT_FAILURE);

	
	while(!done){
		while (SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				done = SDL_TRUE;
			}
			//when the mouse mooves the monster also moves
			if(event.type == SDL_MOUSEMOTION){
				int x = 0, y = 0;

				get_board_place(event.motion.x, event.motion.y, &x, &y);

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
	close_board_windows();
	return EXIT_SUCCESS;
}

void * threadReceive(void *arg){
	int err;
	pos_update *msg = malloc(sizeof(pos_update));
	player *my_player = (player*) arg;

	err = rcv_board(my_player->sock_fd, &board_x, &board_y);
	if (err == -1) exit(EXIT_FAILURE);

	create_board_window(board_x, board_y);

	err = rcv_position(my_player);
	if (err == -1) exit(EXIT_FAILURE);

	while(!done){
		err = recv(my_player->sock_fd, msg , sizeof(&msg), 0);
		if(err == -1){
			perror("receive: ");
			exit(EXIT_FAILURE);
		}
			
		printf("clt rcv pos_update: %d %d %d\n", msg->new_x, msg->new_y, msg->character);

		clear_place(msg->x, msg->y);

		switch (msg->character)
		{
			case MONSTER:  
				paint_monster(msg->new_x, msg->new_y, my_player->p_color->r,my_player->p_color->g, 
								my_player->p_color->b); 
				break;
			case PACMAN: 
				paint_pacman(msg->new_x, msg->new_y, my_player->p_color->r,my_player->p_color->g, 
								my_player->p_color->b); 
				break;	
			case SUPERPACMAN: 
				paint_powerpacman(msg->new_x, msg->new_y, my_player->p_color->r,my_player->p_color->g, 
							my_player->p_color->b); 
				break;
			case LEMON:    
				paint_lemon(msg->new_x, msg->new_y); 
				break;
			case CHERRY: 
				paint_cherry(msg->new_x, msg->new_y); 
				break;
		}
				
    	printf("received %d byte %d %d %d\n", err, msg->character, msg->new_x, msg->new_y);

		/*switch (event.key.keysym.sym)
		{
			case SDLK_LEFT:  x_new--; break;
			case SDLK_RIGHT: x_new++; break;
			case SDLK_UP:    y_new--; break;
			case SDLK_DOWN:  y_new++; break;
		}*/
	}

	return (NULL);
}

/*void * threadSend(void *arg){
	
	struct player *my_player = (struct player *) arg;
	int err, dir;

	err = send_color(my_player->sock_fd, my_player->p_color);
	if(err == -1) exit(EXIT_FAILURE);

	while(!done){
		while (SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				done = SDL_TRUE;
			}
			//when the mouse mooves the monster also moves
			if(event.type == SDL_MOUSEMOTION){
				int x_new, y_new;

				//this fucntion return the place cwher the mouse cursor is
				get_board_place(event.motion .x, event.motion .y,
												&x_new, &y_new);

				err = send_event(MOUSE,  x_new,  y_new,  -1, my_player);
				if(err == -1) exit(EXIT_SUCESS);
				
			}
			if(event.type == SDL_KEYDOWN){

				switch (event.key.keysym.sym)
				{
					case SDLK_LEFT:  dir = LEFT; break;
					case SDLK_RIGHT: dir = RIGHT; break;
					case SDLK_UP:    dir = UP; break;
					case SDLK_DOWN:  dir = DOWN; break;
				}

				err = send_event(MOUSE,  -1,  -1, dir, my_player);
				if(err == -1) exit(EXIT_SUCESS);
			}
		}
	}

	return (NULL);
}*/

