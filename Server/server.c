#include "server.h"


void * serverThread(void *arg){
	return 0;
}

int main(int argc, char* argv[]){

	SDL_Event event;
	int done = 0;

	if(argc != 1){
		printf("error ");
		exit(1);
	}

	//creates a windows and a board with 50x20 cases
	create_board_window(50, 20);

	//monster and packman position
	int x = 0;
	int y = 0;

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	pthread_t *thread_id;

	pthread_create(thread_id, NULL, serverThread, NULL);

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

					paint_pacman(x, y , 200, 100, 7);					
					printf("move x-%d y-%d\n", x,y);
				}
			}
		}
	}
	
	printf("fim\n");
	close_board_windows();
}
