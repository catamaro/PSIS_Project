#include "client.h"

void * clientThread(void *arg){
	return 0;
}

int main(int argc, char* argv[]){

	SDL_Event event;
	int done = 0;
	char *IP;
	char *port;

	if(argc != 3){
		printf("error ");
		exit(1);
	}
	else{
		IP = (char*) malloc(sizeof(argv[1]));
		port = (char*) malloc(sizeof(argv[2]));
		strcpy(IP, argv[1]);
		strcpy(port, argv[2]);
	}

	//creates a windows and a board with 50x20 cases
	create_board_window(50, 20);

	//monster and packman position
	int x = 0;
	int y = 0;

	pthread_t *thread_id;

	pthread_create(thread_id, NULL, clientThread, NULL);

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
