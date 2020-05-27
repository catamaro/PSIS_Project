#include "structs.h"
#include "server.h"

int sock_fd; 
int board_x;
int board_y;

#define MAX_PLAYERS_WAITING 5

int main(int argc, char* argv[]){
	
	SDL_Event event;
	int done = 0;
	struct sockaddr_in local_addr;
	

	if(argc != 2){
		printf("error: wrong arguments\n");
		exit(1);
	}

	int **board = loadBoard(argv[1]);

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1){
		perror("socket: ");
		exit(EXIT_FAILURE);
	}

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = htons(58000);
	int err = bind(sock_fd, (struct sockaddr *)&local_addr, 
						sizeof(local_addr));
	if(err == -1){
		perror("bind: ");
		exit(EXIT_FAILURE);
	}

	printf(" socket created and binded \n");
	if(listen(sock_fd, MAX_PLAYERS_WAITING) == -1){
		perror("listen: ");
		exit(EXIT_FAILURE);
	}

	//accept thread id
	pthread_t thread_id;

	//accepts new player connections
	pthread_create(&thread_id, NULL, threadAccept, NULL);

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

					paint_pacman(x, y , 200, 100, 7);					
					printf("move x-%d y-%d\n", x,y);
				}
			}
		}
	}

	free(board[0]);
    free(board);

	printf("fim\n");
	close_board_windows();

	exit(EXIT_SUCCESS);
}


void * threadAccept(void *arg){
	struct sockaddr_in client_addr;
	socklen_t size_addr = sizeof(client_addr);
	player *new_player = (player*) malloc(sizeof(player));
	int num_players = 0;

	while(1){
		printf("waiting for players\n");
		sock_fd = accept(sock_fd,
							(struct sockaddr *) & client_addr, &size_addr);
		if(sock_fd == -1) {
			perror("accept ");
			exit(EXIT_FAILURE);
		}

		new_player->id = num_players;
		new_player->x = random()%board_x;
		new_player->x = random()%board_y;
		num_players ++;
	}

	return (NULL);
}

void * threadClient(void *arg){

	return (NULL);
}

int ** loadBoard(char* arg){
	FILE * fp;
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int i;

	// Name of file will be an input of main later
    fp = fopen(arg, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // Read number of lines and columns of board
    if(fscanf(fp,"%d %d",&board_y,&board_x)!=2)
    {
      printf("Please give a valid input file.");
      exit(EXIT_FAILURE);
    }

	// Create contiguous 2D array to store bricks/board in general
    int *temp = calloc(board_x * board_y, sizeof(int));
    int **board = calloc(board_x, sizeof(int*));
    board[0] = temp;
    for(i = 1; i < board_x; i++)
      board[i] = board[i-1] + board_y;

	//creates a windows and a board
	create_board_window(board_x, board_y);

    // Consume first line, that is correct due to previous fscanf
    getline(&line, &len, fp);

    // Read file line by line and store location of brick in board[][]
    i = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
        // If line doesn't have board_y columns
        if(read!=(board_y+1))
        {
          printf("Please give a valid input file.\n");
          exit(EXIT_FAILURE);
        }
        // For a given line, iterate all the columns
        for(int j = 0; j < board_y; j++)
        {
          // If there is a brick, store in board
          if(line[j] == 'B'){
			board[i][j] = 1;
			paint_brick(i, j);
		  }
        }
        // i is the index of current line
        i++;
    }

    // If file doesnt have board_x lines
    if(i!=(board_x))
    {
      printf("Please give a valid input file.\n");
      exit(EXIT_FAILURE);
    }

    // Close file, free line reader
    fclose(fp);
    if(line) free(line);

	return board;
}
