#include "client.h"
#include "structs.h"
#include "comm.h"

int board_x;
int board_y;

int main(int argc, char* argv[]){

	SDL_Event event;
	int done = 0;
	char *IP;
	int port;
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
	pthread_t send_id;

	//receives messages from server
	pthread_create(&receive_id, NULL, threadReceive, (void *)&my_player->sock_fd);
	//send messages to server
	pthread_create(&send_id, NULL, threadSend, (void *)my_player);
	
	//monster and packman position
	int x = 0;
	int y = 0;

	while (!done){
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
				//if the mluse moved toi anothe place
				if((x_new != x) || (y_new != y)){
					//the old place is cleared
					clear_place(x, y);
					x = x_new;
					y = y_new;
	
					paint_monster(x, y , 200, 100, 7);
		
					printf("move x-%d y-%d\n", x,y);
				}
			}
			if(event.type == SDL_KEYDOWN){
				int x_new, y_new;

				if (event.key.keysym.sym == SDLK_LEFT ){
					//the olde place is cleared
					clear_place(x, y);

					//paint the pacman
					paint_pacman(x, y , 7, 100, 200);
					printf("move x-%d y-%d\n", x,y);
				}
			}
		}
	}
	
	printf("fim\n");
	close(my_player->sock_fd);
	close_board_windows();
}

void * threadReceive(void *arg){
	int err;
	pos_update msg;
	int *sock_fd = (int*) arg;
	char first_msg[128];

	err = recv(*sock_fd, &first_msg , sizeof(first_msg), 0);
	if (err == -1){
		perror("receive: ");
		exit(EXIT_FAILURE);
	} 

	err = rcv_board(first_msg, &board_x, &board_y);
	if (err == -1) exit(EXIT_FAILURE);

	create_board_window(board_x, board_y);

	while((err = recv(*sock_fd, &msg , sizeof(msg), 0)) > 0){
		/* code to update position from other players */ 
    	printf("received %d byte %d %d %d\n", err, msg.character, msg.x, msg.y);
	}

	return (NULL);
}

void * threadSend(void *arg){
	
	struct player *my_player = (struct player *) arg;
	int err;

	err = send_color(my_player->sock_fd, my_player->p_color);
	if(err == -1) exit(EXIT_FAILURE);

	return (NULL);
}

