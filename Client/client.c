#include "client.h"

void * clientThread(void *arg){
	

	return (NULL);
}

int main(int argc, char* argv[]){

	SDL_Event event;
	int done = 0;
	char *IP;
	int port;
	struct sockaddr_in server_addr;


	if(argc != 3){
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
	}
	
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1){
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

	err = connect(sock_fd, (const struct sockaddr *)&server_addr,
						sizeof(server_addr));
	if(err == -1){
		perror("bind: ");
		exit(EXIT_FAILURE);
	}

	int err_rcv;
	exe4_message msg;
	
	printf("just connected to the server \n");

	while((err_rcv = recv(sock_fd, &msg , sizeof(msg), 0)) >0 ){
    	printf("received %d byte %d %d %d\n", err_rcv, msg.character, msg.x, msg.y);
	}

	pthread_t *thread_id;

	pthread_create(thread_id, NULL, clientThread, NULL);

	//creates a windows and a board with 50x20 cases
	create_board_window(50, 20);

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
		}
	}
	
	printf("fim\n");
	close_board_windows();
}
