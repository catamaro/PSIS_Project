#include "structs.h"
#include "server.h"
#include "list_handler.h"
#include "comm.h"


int server_fd; 
int board_x;
int board_y;

#define MAX_PLAYERS_WAITING 5
#define MAX_PLAYERS board_x*board_y/2 //provisório

int main(int argc, char* argv[]){
	
	SDL_Event event;
	int done = 0;
	struct sockaddr_in local_addr;
	

	if(argc != 2){
		printf("error: wrong arguments\n");
		exit(EXIT_FAILURE);
	}

	int **board = loadBoard(argv[1]);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd == -1){
		perror("socket: ");
		exit(EXIT_FAILURE);
	}

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = htons(58008);
	int err = bind(server_fd, (struct sockaddr *)&local_addr, 
						sizeof(local_addr));
	if(err == -1){
		perror("bind: ");
		exit(EXIT_FAILURE);
	}

	printf("Socket created and binded \n");
	if(listen(server_fd, MAX_PLAYERS_WAITING) == -1){
		perror("listen: ");
		exit(EXIT_FAILURE);
	}

	//accept thread id
	pthread_t thread_id;

	//accepts new player connections
	pthread_create(&thread_id, NULL, threadAccept, NULL);

	while (!done){
		while (SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				done = SDL_TRUE;
			}
			
		}
	}

	free(board[0]);
    free(board);
	freeList();

	printf("fim\n");
	close_board_windows();
	close(server_fd);

	exit(EXIT_SUCCESS);
}


void * threadAccept(void *arg){
	struct sockaddr_in client_addr;
	socklen_t size_addr = sizeof(client_addr);

	struct position *pos1 = malloc(sizeof(position));
	struct position *pos2 = malloc(sizeof(position));
	int num_players = 0, new_fd;
	struct player *new_player;
	size_t err;


	while(1){
		printf("waiting for players\n");
		new_fd = accept(server_fd,
							(struct sockaddr *) & client_addr, &size_addr);
		if(new_fd == -1) {
			perror("accept ");
			exit(EXIT_FAILURE);
		}
		
		if(num_players > MAX_PLAYERS){
			printf("Maximum number of players achived\n");
			close(new_fd);
			continue;
		}

		err = rcv_color(new_fd);
		if(err == -1){
			printf("error: color already in use\n");
			close(new_fd);
			continue;
		}
		
		// provisório
		pos1->x = random()%board_x;
		pos1->y = random()%board_y;
		pos2->x = random()%board_x;
		pos2->y = random()%board_y;

		new_player = insertPlayer(pos1, pos2, num_players, new_fd);

		pthread_create(&(new_player->thread_id), NULL, threadClient, (void *)new_player->sock_fd);

		printf("Player %d entered the game\n", num_players);

		err = send_board(board_x, board_y, new_player);
		if(err == -1) exit(EXIT_FAILURE);

		num_players ++;
	}

	return (NULL);
}

void * threadClient(void *arg){
	int player_fd = (player *) arg;
	pos_update *new_update = malloc(sizeof(pos_update));
	SDL_Event event;
	int err;

	while(err = rcv_event(player_fd, &event) > 0){
		SDL_PushEvent(event);
	}

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
