#include "client.h"

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
	printf("just connected to the server \n");
	
	//receive and send thread id
	pthread_t receive_id;
	pthread_t send_id;

	//receives messages from server
	pthread_create(&receive_id, NULL, threadReceive, (void *)&sock_fd);
	//send messages to server
	pthread_create(&send_id, NULL, threadSend, (void *)&sock_fd);
	
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
	close(sock_fd);
	close_board_windows();
}

void * threadReceive(void *arg){
	int err_rcv;
	exe4_message msg;
	int *sock_fd = (int*) arg;

	printf("%d\n", *sock_fd);

	while((err_rcv = recv(*sock_fd, &msg , sizeof(msg), 0)) > 0 ){
    	printf("received %d byte %d %d %d\n", err_rcv, msg.character, msg.x, msg.y);
	}

	return (NULL);
}

void * threadSend(void *arg){

	return (NULL);
}