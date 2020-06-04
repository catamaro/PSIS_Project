#include "structs.h"
#include "server.h"
#include "list_handler.h"
#include "comm.h"

int server_fd;
int board_x;
int board_y;
int empty_blocks;
int num_bricks;
int num_fruits = 0;
int num_players = 0;
int **board;
int done = 0;

unsigned int Event_MovePacman = 0;
unsigned int Event_MoveMonster = 1;

#define MAX_PLAYERS (int)floor((board_x * board_y - num_bricks + 2) / 4)

int main(int argc, char *argv[])
{

	SDL_Event event;
	struct sockaddr_in local_addr;

	if (argc != 2)
	{
		printf("error: wrong arguments\n");
		exit(EXIT_FAILURE);
	}

	board = loadBoard(argv[1]);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		perror("socket: ");
		exit(EXIT_FAILURE);
	}

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = htons(58000);
	int err = bind(server_fd, (struct sockaddr *)&local_addr,
				   sizeof(local_addr));
	if (err == -1)
	{
		perror("bind: ");
		exit(EXIT_FAILURE);
	}

	printf("Socket created and binded \n");
	if (listen(server_fd, MAX_PLAYERS_WAITING) == -1)
	{
		perror("listen: ");
		exit(EXIT_FAILURE);
	}

	//accept thread id
	pthread_t thread_id;

	//accepts new player connections
	pthread_create(&thread_id, NULL, threadAccept, NULL);

	int x_new, y_new, x, y, x_new_2, y_new_2;
	struct player *player1, *player2;
	struct position *new_position;

	while (!done)
	{
		SDL_Delay(20);
		while (SDL_PollEvent(&event))
		{

			ManageFruits();
			if (event.type == SDL_QUIT)
			{
				done = SDL_TRUE;
			}
			if (event.type == Event_MovePacman || event.type == Event_MoveMonster)
			{
				player1 = event.user.data1;
				new_position = event.user.data2;

				if (event.type == Event_MoveMonster)
				{
					x = player1->monster->x;
					y = player1->monster->y;
				}
				else
				{
					x = player1->pacman->x;
					y = player1->pacman->y;
				}

				x_new = new_position->x;
				y_new = new_position->y;

				if (x == x_new && y == y_new)
					continue;

				if (event.type == Event_MoveMonster)
				{
					player2 = findPlayerPos(x_new, y_new, MONSTER);
					printf("x: %d y: %d\n", x_new, y_new);

					// Test for bricks and out of bound
					bounceBounds(x, y, &x_new, &y_new);
					checkRulesMonster(player1, player2, &x_new, &y_new, &x_new_2, &y_new_2);
					broadcast_update(x_new, y_new, x, y, MONSTER, player1->rgb);
					if(x_new_2 != -1) broadcast_update(x_new, y_new, x, y, MONSTER, player2->rgb);
				}
				else
				{

					player2 = findPlayerPos(x_new, y_new, PACMAN);
					printf("x: %d y: %d\n", x_new, y_new);

					// Test for bricks and out of bound
					bounceBounds(x, y, &x_new, &y_new);
					if (board[x][y] == PACMAN)
					{
						checkRulesPacman(player1, player2,&x_new, &y_new, &x_new_2, &y_new_2);
						broadcast_update(x_new, y_new, x, y, PACMAN, player1->rgb);
					}

					else if (board[x][y] == SUPERPACMAN)
					{
						checkRulesSuperPacman(player1, player2, &x_new, &y_new, &x_new_2, &y_new_2);
						broadcast_update(x_new, y_new, x, y, SUPERPACMAN, player1->rgb);
						
					}
				}
			}
		}
	}

	free(board[0]);
	free(board);
	freeList();
	freePosList();

	printf("fim\n");
	close_board_windows();
	close(server_fd);

	exit(EXIT_SUCCESS);
}

void *threadAccept(void *arg)
{
	struct sockaddr_in client_addr;
	socklen_t size_addr = sizeof(client_addr);
	int new_fd;
	struct player *new_player;
	size_t err;

	while (!done)
	{
		struct color *new_color = malloc(sizeof(color));
		struct position *pacman = malloc(sizeof(position));
		struct position *monster = malloc(sizeof(position));

		printf("waiting for players\n");
		new_fd = accept(server_fd,
						(struct sockaddr *)&client_addr, &size_addr);
		if (new_fd == -1)
		{
			perror("accept ");
			exit(EXIT_FAILURE);
		}

		if (num_players > MAX_PLAYERS)
		{
			printf("Maximum number of players achived\n");
			close(new_fd);
			continue;
		}

		err = rcv_color(new_fd, new_color);
		if (err == -1)
		{
			printf("error: color already in use\n");
			close(new_fd);
			continue;
		}
		// get random position on the board
		RandomPositionConnect(&(pacman->x), &(pacman->y), &(monster->x), &(monster->y));
		// update board with position
		board[pacman->x][pacman->y] = PACMAN;
		board[monster->x][monster->y] = MONSTER;
		// paint both characters in server board
		paint_pacman(pacman->x, pacman->y, new_color->r, new_color->g, new_color->b);
		paint_monster(monster->x, monster->y, new_color->r, new_color->g, new_color->b);

		// add new player to player list
		new_player = insertPlayer(pacman, monster, new_color, num_players, new_fd);
		
		// creeate thread for new client
		pthread_create(&(new_player->thread_id), NULL, threadClient, (void *)&(new_player->sock_fd));

		printf("Player %d entered the game\n", num_players);
		// send board dimensions to new client
		err = send_board_dim(board_x, board_y, new_player->sock_fd);
		sleep(1);
		if (err == -1)
			exit(EXIT_FAILURE);

		err = send_board_setup(new_player->sock_fd);
		if (err == -1)
			exit(EXIT_FAILURE);

		num_players++;

		ManageFruits();
	}

	return (NULL);
}

void *threadClient(void *arg)
{
	int *player_fd = (int *)arg;
	int err, character;
	SDL_Event new_event;

	while (!done)
	{
		SDL_zero(new_event);

		err = rcv_event(*player_fd, &new_event, &character);
		if (err == -1)
		{
			close(*player_fd);
			return (NULL);
		}
		if (character == MONSTER)
			new_event.type = Event_MoveMonster;
		if (character == PACMAN)
			new_event.type = Event_MovePacman;

		SDL_PushEvent(&new_event);
	}

	return (NULL);
}

int **loadBoard(char *arg)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int i;

	// Name of file will be an input of main later
	fp = fopen(arg, "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	// Read number of lines and columns of board
	if (fscanf(fp, "%d %d", &board_y, &board_x) != 2)
	{
		printf("Please give a valid input file.");
		exit(EXIT_FAILURE);
	}

	// Create contiguous 2D array to store bricks/board in general
	int *temp = calloc(board_x * board_y, sizeof(int));
	int **board = calloc(board_x, sizeof(int *));
	board[0] = temp;
	for (i = 1; i < board_x; i++)
		board[i] = board[i - 1] + board_y;

	//creates a windows and a board
	create_board_window(board_x, board_y);

	// Consume first line, that is correct due to previous fscanf
	read = getline(&line, &len, fp);

	// Read file line by line and store location of brick in board[][]
	i = 0;
	while ((read = getline(&line, &len, fp)) != -1)
	{
		// If line doesn't have board_y columns
		if (read != (board_y + 1))
		{
			printf("Please give a valid input file.\n");
			exit(EXIT_FAILURE);
		}
		// For a given line, iterate all the columns
		for (int j = 0; j < board_y; j++)
		{
			// If there is a brick, store in board
			if (line[j] == 'B')
			{
				board[i][j] = 1;
				paint_brick(i, j);
				AddPosHead(i, j, BRICK);
			}
		}
		// i is the index of current line
		i++;
	}

	// If file doesnt have board_x lines
	if (i != (board_x))
	{
		printf("Please give a valid input file.\n");
		exit(EXIT_FAILURE);
	}

	// Close file, free line reader
	fclose(fp);
	if (line)
		free(line);

	return board;
}

void ManageFruits()
{
	if (num_players <= 1)
		return;
	if (num_fruits == (num_players - 1) * 2)
		return;
	if (num_fruits > (num_players - 1) * 2)
	{
		do
		{
			int x, y;
			FetchFruitHeadCoords(&x, &y);
			RemoveFruitHead();
			clear_place(x, y);
			board[x][y] = 0;
			num_fruits--;
		} while (num_fruits > (num_players - 1) * 2);
		return;
	}
	do
	{
		int x, y;
		RandomPositionRules(&x, &y);

		int fruit = rand() % (CHERRY + 1 - LEMON) + LEMON;
		AddPosHead(x, y, fruit);

		num_fruits++;
		if (fruit == LEMON)
		{
			board[x][y] = LEMON;
			paint_lemon(x, y);
		}
		else if (fruit == CHERRY)
		{
			board[x][y] = CHERRY;
			paint_cherry(x, y);
		}
	} while (num_fruits < (num_players - 1) * 2);
}

void checkRulesMonster(struct player *dealer, struct player *receiver, int *x_new, 
						int *y_new, int *x_new_2, int *y_new_2)
{
	// Monster moving to empty place
	if (board[*x_new][*y_new] == EMPTY)
	{
		int old_x = dealer->monster->x;
		int old_y = dealer->monster->y;
		board[old_x][old_y] = EMPTY;
		clear_place(old_x, old_y);
		dealer->monster->x = *x_new;
		dealer->monster->y = *y_new;
		board[*x_new][*y_new] = MONSTER;
		paint_monster(*x_new, *y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
	}
	// Monster moving to pacman
	else if (board[*x_new][*y_new] == PACMAN)
	{
		// Same person's pacman
		if (dealer->id == receiver->id)
		{
			// Save values in integers to avoid deffering many pointers
			int old_x1 = receiver->pacman->x;
			int old_y1 = receiver->pacman->y;
			int old_x2 = dealer->monster->x;
			int old_y2 = dealer->monster->y;

			clear_place(old_x1, old_y1);
			clear_place(old_x2, old_y2);

			// Put monster in pacman's block
			dealer->monster->x = old_x1;
			dealer->monster->y = old_y1;
			// Put pacman in monster's block
			receiver->pacman->x = old_x2;
			receiver->pacman->y = old_y2;
			board[old_x2][old_y2] = PACMAN;
			board[old_x1][old_y1] = MONSTER;

			paint_pacman(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
			paint_monster(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		}
		// Another person's pacman
		else
		{
			// Save values in integers to avoid deffering many pointers
			int old_x1 = receiver->pacman->x;
			int old_y1 = receiver->pacman->y;
			int old_x2 = dealer->monster->x;
			int old_y2 = dealer->monster->y;
			// Clean old pacman space
			board[old_x2][old_y2] = EMPTY;
			clear_place(old_x2, old_y2);
			// Save new monster coordinates, increment score and paint
			dealer->monster->x = old_x1;
			dealer->monster->y = old_y1;
			dealer->score = dealer->score + 1;
			board[old_x1][old_y1] = MONSTER;
			paint_monster(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
			// Fetch new pacman coordinates, save and paint
			RandomPositionRules(&old_x1, &old_y1);
			board[old_x1][old_y1] = PACMAN;
			receiver->pacman->x = old_x1;
			receiver->pacman->y = old_y1;
			paint_pacman(old_x1, old_y1, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
		}
	}
	// Monster moving to superpacman
	else if (board[*x_new][*y_new] == SUPERPACMAN)
	{
		// Same person's superpacman
		if (dealer->id == receiver->id)
		{
			// Save values in integers to avoid deffering many pointers
			int old_x1 = receiver->pacman->x;
			int old_y1 = receiver->pacman->y;
			int old_x2 = dealer->monster->x;
			int old_y2 = dealer->monster->y;

			clear_place(old_x1, old_y1);
			clear_place(old_x2, old_y2);

			// Put monster in superpacman's block
			dealer->monster->x = old_x1;
			dealer->monster->y = old_y1;
			// Put superpacman in monster's block
			receiver->pacman->x = old_x2;
			receiver->pacman->y = old_y2;

			board[old_x2][old_y2] = SUPERPACMAN;
			board[old_x1][old_y1] = MONSTER;

			paint_pacman(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
			paint_monster(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		}

		else
		{
			// Another person's superpacman
			int old_x1 = receiver->pacman->x;
			int old_y1 = receiver->pacman->y;
			int old_x2 = dealer->monster->x;
			int old_y2 = dealer->monster->y;
			// Increment score for superpacman
			receiver->score = receiver->score + 1;
			// Decrement number of times superpacman can eat
			receiver->times = receiver->times - 1;
			// If superpacman ate 2 monsters, go back to normal pacman
			if (receiver->times < 1)
			{
				clear_place(old_x1, old_y1);
				board[old_x1][old_y1] = PACMAN;
				paint_pacman(old_x1, old_y1, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
			}
			// Clean old monster coordinates
			board[old_x2][old_y2] = EMPTY;
			clear_place(old_x2, old_y2);
			// New monster coordinates
			RandomPositionRules(&old_x2, &old_y2);
			board[old_x2][old_y2] = MONSTER;
			dealer->monster->x = old_x2;
			dealer->monster->y = old_y2;
			paint_monster(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		}
	}
	// Monster moving to another monster
	else if (board[*x_new][*y_new] == MONSTER)
	{
		int old_x1 = receiver->monster->x;
		int old_y1 = receiver->monster->y;
		int old_x2 = dealer->monster->x;
		int old_y2 = dealer->monster->y;
		clear_place(old_x1, old_y1);
		clear_place(old_x2, old_y2);
		// Swap monsters
		dealer->monster->x = old_x1;
		dealer->monster->y = old_y1;
		receiver->monster->x = old_x2;
		receiver->monster->y = old_y2;
		paint_monster(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		paint_monster(old_x2, old_y2, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
	}
	// Monster moving to fruit
	else if (board[*x_new][*y_new] == LEMON || board[*x_new][*y_new] == CHERRY)
	{
		int old_x2 = dealer->monster->x;
		int old_y2 = dealer->monster->y;
		dealer->score = dealer->score + 1;
		num_fruits--;
		RemoveFruitPosition(*x_new, *y_new);
		board[old_x2][old_y2] = EMPTY;
		clear_place(old_x2, old_y2);
		clear_place(*x_new, *y_new);
		dealer->monster->x = *x_new;
		dealer->monster->y = *y_new;
		board[*x_new][*y_new] = MONSTER;
		paint_monster(*x_new, *y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
	}
}

void checkRulesPacman(struct player *dealer, struct player *receiver, int *x_new, 
						int *y_new, int *x_new_2, int *y_new_2)
{
	// Pacman moving to empty place
	if (board[*x_new][*y_new] == EMPTY)
	{
		int old_x = dealer->pacman->x;
		int old_y = dealer->pacman->y;
		board[old_x][old_y] = EMPTY;
		clear_place(old_x, old_y);
		dealer->pacman->x = *x_new;
		dealer->pacman->y = *y_new;
		board[*x_new][*y_new] = PACMAN;
		paint_pacman(*x_new, *y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
	}
	// Pacman moving to monster
	else if (board[*x_new][*y_new] == MONSTER)
	{
		printf("TestPM\n");
		// Same person's monster
		if (dealer->id == receiver->id)
		{
			// Save values in integers to avoid deffering many pointers
			int old_x1 = receiver->monster->x;
			int old_y1 = receiver->monster->y;
			int old_x2 = dealer->pacman->x;
			int old_y2 = dealer->pacman->y;

			clear_place(old_x1, old_y1);
			clear_place(old_x2, old_y2);

			// Put pacman in monster's block
			dealer->pacman->x = old_x1;
			dealer->pacman->y = old_y1;
			// Put monster in pacman's block
			receiver->monster->x = old_x2;
			receiver->monster->y = old_y2;

			board[old_x2][old_y2] = MONSTER;
			board[old_x1][old_y1] = PACMAN;

			paint_monster(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
			paint_pacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		}
		// Another person's monster
		else
		{
			int old_x2 = dealer->pacman->x;
			int old_y2 = dealer->pacman->y;
			// Increment score for monster
			receiver->score = receiver->score + 1;
			// Clean old pacman coordinates
			board[old_x2][old_y2] = EMPTY;
			clear_place(old_x2, old_y2);
			// New pacman coordinates
			RandomPositionRules(&old_x2, &old_y2);
			board[old_x2][old_y2] = PACMAN;
			dealer->pacman->x = old_x2;
			dealer->pacman->y = old_y2;
			paint_pacman(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		}
	}
	// Pacman moving to superpacman
	else if (board[*x_new][*y_new] == SUPERPACMAN)
	{
		int old_x1 = receiver->pacman->x;
		int old_y1 = receiver->pacman->y;
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		clear_place(old_x1, old_y1);
		clear_place(old_x2, old_y2);
		// Swap pacman and superpacman
		dealer->pacman->x = old_x1;
		dealer->pacman->y = old_y1;
		receiver->pacman->x = old_x2;
		receiver->pacman->y = old_y2;
		paint_pacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		paint_powerpacman(old_x2, old_y2, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
	}
	// Pacman moving to another pacman
	else if (board[*x_new][*y_new] == PACMAN)
	{
		int old_x1 = receiver->pacman->x;
		int old_y1 = receiver->pacman->y;
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		clear_place(old_x1, old_y1);
		clear_place(old_x2, old_y2);
		// Swap pacmans
		dealer->pacman->x = old_x1;
		dealer->pacman->y = old_y1;
		receiver->pacman->x = old_x2;
		receiver->pacman->y = old_y2;
		paint_pacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		paint_pacman(old_x2, old_y2, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
	}
	// Pacman moving to fruit
	else if (board[*x_new][*y_new] == LEMON || board[*x_new][*y_new] == CHERRY)
	{
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		dealer->score = dealer->score + 1;
		num_fruits--;
		RemoveFruitPosition(*x_new, *y_new);
		board[old_x2][old_y2] = EMPTY;
		clear_place(old_x2, old_y2);
		clear_place(*x_new, *y_new);
		dealer->pacman->x = *x_new;
		dealer->pacman->y = *y_new;
		dealer->times = 2;
		board[*x_new][*y_new] = SUPERPACMAN;
		paint_powerpacman(*x_new, *y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
	}
}

void checkRulesSuperPacman(struct player *dealer, struct player *receiver, int *x_new, 
						int *y_new, int *x_new_2, int *y_new_2)
{
	// Superpacman moving to empty place
	if (board[*x_new][*y_new] == EMPTY)
	{
		int old_x = dealer->pacman->x;
		int old_y = dealer->pacman->y;
		board[old_x][old_y] = EMPTY;
		clear_place(old_x, old_y);
		dealer->pacman->x = *x_new;
		dealer->pacman->y = *y_new;
		board[*x_new][*y_new] = SUPERPACMAN;
		paint_powerpacman(*x_new, *y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
	}
	// Superpacman moving to monster
	else if (board[*x_new][*y_new] == MONSTER)
	{
		printf("TestSPM\n");
		// Same person's monster
		if (dealer->id == receiver->id)
		{
			// Save values in integers to avoid deffering many pointers
			int old_x1 = receiver->monster->x;
			int old_y1 = receiver->monster->y;
			int old_x2 = dealer->pacman->x;
			int old_y2 = dealer->pacman->y;

			clear_place(old_x1, old_y1);
			clear_place(old_x2, old_y2);

			// Put superpacman in monster's block
			dealer->pacman->x = old_x1;
			dealer->pacman->y = old_y1;
			// Put monster in superpacman's block
			receiver->monster->x = old_x2;
			receiver->monster->y = old_y2;

			board[old_x2][old_y2] = MONSTER;
			board[old_x1][old_y1] = SUPERPACMAN;

			paint_monster(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
			paint_powerpacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		}
		// Another person's monster
		else
		{
			int old_x1 = receiver->monster->x;
			int old_y1 = receiver->monster->y;
			int old_x2 = dealer->pacman->x;
			int old_y2 = dealer->pacman->y;
			// Increment score for superpacman
			dealer->score = dealer->score + 1;
			// Decrement number of times superpacman can eat
			dealer->times = dealer->times - 1;
			// If superpacman ate 2 monsters, go back to normal pacman
			if (dealer->times < 1)
			{
				clear_place(old_x2, old_y2);
				board[old_x2][old_y2] = PACMAN;
				paint_pacman(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
			}
			else
			{
				clear_place(old_x2, old_y2);
				board[old_x2][old_y2] = SUPERPACMAN;
				paint_powerpacman(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
			}
			// Clean old monster coordinates
			board[old_x1][old_y1] = EMPTY;
			clear_place(old_x1, old_y1);
			// New monster coordinates
			RandomPositionRules(&old_x1, &old_y1);
			board[old_x1][old_y1] = MONSTER;
			receiver->monster->x = old_x1;
			receiver->monster->y = old_y1;
			paint_monster(old_x1, old_y1, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
		}
	}
	// Superpacman moving to another superpacman
	else if (board[*x_new][*y_new] == SUPERPACMAN)
	{
		int old_x1 = receiver->pacman->x;
		int old_y1 = receiver->pacman->y;
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		clear_place(old_x1, old_y1);
		clear_place(old_x2, old_y2);
		// Swap pacmans
		dealer->pacman->x = old_x1;
		dealer->pacman->y = old_y1;
		receiver->pacman->x = old_x2;
		receiver->pacman->y = old_y2;
		paint_powerpacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		paint_powerpacman(old_x2, old_y2, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
	}
	// Superpacman moving to a pacman
	else if (board[*x_new][*y_new] == PACMAN)
	{
		int old_x1 = receiver->pacman->x;
		int old_y1 = receiver->pacman->y;
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		clear_place(old_x1, old_y1);
		clear_place(old_x2, old_y2);
		// Swap pacmans
		dealer->pacman->x = old_x1;
		dealer->pacman->y = old_y1;
		receiver->pacman->x = old_x2;
		receiver->pacman->y = old_y2;
		paint_powerpacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		paint_pacman(old_x2, old_y2, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
	}
	// Superpacman moving to fruit
	else if (board[*x_new][*y_new] == LEMON || board[*x_new][*y_new] == CHERRY)
	{
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		dealer->score = dealer->score + 1;
		num_fruits--;
		RemoveFruitPosition(*x_new, *y_new);
		board[old_x2][old_y2] = EMPTY;
		clear_place(old_x2, old_y2);
		clear_place(*x_new, *y_new);
		dealer->pacman->x = *x_new;
		dealer->pacman->y = *y_new;
		board[*x_new][*y_new] = SUPERPACMAN;
		paint_powerpacman(*x_new, *y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
	}
}

void bounceBounds(int x1, int y1, int *x_new, int *y_new)
{
	// Moving out of bounds in x
	if (*x_new >= board_x)
		*x_new = board_x - 2;
	else if (*x_new < 0)
		*x_new = 1;
	// Moving out of bounds in y
	else if (*y_new >= board_y)
		*y_new = board_y - 2;
	else if (*y_new < 0)
		*y_new = 1;
	// If moving to a brick
	else if (board[*x_new][*y_new] == BRICK)
	{
		// If moving from left to block
		if (*x_new > x1 && *y_new == y1)
		{
			// If bounce would make it out of bounds
			if (*x_new - 2 < 0)
				*x_new = 0;
			// If bounce would put on another brick
			else if (board[*x_new - 2][*y_new] == BRICK)
				*x_new = x1; // Stay on the same place
			// Usual bounce
			else
				*x_new = *x_new - 2; // Move 2 to the the left of the block
		}
		// If moving from right to block
		else if (*x_new < x1 && *y_new == y1)
		{
			// If bounce would make it out of bounds
			if (*x_new + 2 >= board_x)
				*x_new = board_x - 1;
			// If bounce would put on another brick
			else if (board[*x_new + 2][*y_new] == BRICK)
				*x_new = x1; // Stay on the same place
			// Usual bounce
			else
				*x_new = *x_new + 2;
		}

		// If moving from top to block
		else if (*y_new > y1 && *x_new == x1)
		{
			// If bounce would make it out of bounds
			if (*y_new - 2 < 0)
				*y_new = 0;
			// If bounce would put on another brick
			else if (board[*x_new][*y_new - 2] == BRICK)
				*y_new = y1; // Stay on the same place
			// Usual bounce
			else
				*y_new = *y_new - 2; // Move 2 to the the left of the block
		}
		// If moving from bottom to block
		else if (*y_new < y1 && *x_new == x1)
		{
			// If bounce would make it out of bounds
			if (*y_new + 2 >= board_y)
				*y_new = board_y - 1;
			// If bounce would put on another brick
			else if (board[*x_new][*y_new + 2] == BRICK)
				*y_new = y1; // Stay on the same place
			// Usual bounce
			else
				*y_new = *y_new + 2;
		}
	}
}

void RandomPositionRules(int *x, int *y)
{
	int k, i, j, randomID;
	empty_blocks = board_x * board_y * 0.8;	 // Temporary number of empty blocks
	int threshold = board_x * board_y * 0.5; // Temporary threshold/change percentage
	if (empty_blocks > threshold)			 // While there are many empty blocks
	{
		// New coordinates
		*x = random() % board_x;
		*y = random() % board_y;
		while (board[*x][*y] != EMPTY) // Guarantee that it only finds an empty block
		{
			*x = random() % board_x;
			*y = random() % board_y;
		}
	}
	else // If there aren't many empty blocks
	{
		struct position *empty_ID = malloc(empty_blocks * sizeof(position));
		k = 0; // Reset incremental variable for array

		for (i = 0; i < board_x; i++)
		{
			for (j = 0; j < board_y; j++)
			{
				/* k < empty_blocks is only a temporary condition to assure it doesn't
					 go out out bounds.
					 Find empty blocks and store the coordinates in an array
					 Randomly choose one of those empty blocks
				*/
				if (board[i][j] == EMPTY && k < empty_blocks)
				{
					(empty_ID[k]).x = i;
					(empty_ID[k]).y = j;
					k++;
				}
			}
		}
		// New coordinates
		randomID = (int)random() % empty_blocks;
		*x = (empty_ID[randomID]).x;
		*y = (empty_ID[randomID]).y;
		free(empty_ID);
	}
}

void RandomPositionConnect(int *x1, int *x2, int *y1, int *y2)
{
	int k, i, j, randomID, randomID2;
	empty_blocks = board_x * board_y * 0.8;	 // Temporary number of empty blocks
	int threshold = board_x * board_y * 0.5; // Temporary threshold/change percentage
	if (empty_blocks > threshold)			 // While there are many empty blocks
	{
		// Pacman coordinates
		*x1 = random() % board_x;
		*y1 = random() % board_y;
		while (board[*x1][*y1] != EMPTY) // Guarantee that it only finds an empty block
		{
			*x1 = random() % board_x;
			*y1 = random() % board_y;
		}
		// Monster coordinates
		*x2 = random() % board_x;
		*y2 = random() % board_y;
		// Guarantee that it only finds an empty block and that the monster
		// has different coordinates from the Pacman
		while (board[*x2][*y2] != EMPTY || ((*x1 == *x2) && (*y1 == *y2)))
		{
			*x2 = random() % board_x;
			*y2 = random() % board_y;
		}
	}
	else // If there aren't many empty blocks
	{
		struct position *empty_ID = malloc(empty_blocks * sizeof(position));
		k = 0; // Reset incremental variable for array

		for (i = 0; i < board_x; i++)
		{
			for (j = 0; j < board_y; j++)
			{
				/* k < empty_blocks is only a temporary condition to assure it doesn't
					 go out out bounds.
					 Find empty blocks and store the coordinates in an array
					 Randomly choose one of those empty blocks
				*/
				if (board[i][j] == EMPTY && k < empty_blocks)
				{
					(empty_ID[k]).x = i;
					(empty_ID[k]).y = j;
					k++;
				}
			}
		}
		// Pacman coordinates
		randomID = (int)random() % empty_blocks;
		*x1 = (empty_ID[randomID]).x;
		*y1 = (empty_ID[randomID]).y;
		// Monster coordinates
		randomID2 = random() % empty_blocks;
		while (randomID2 == randomID) // Guarantee coordinates are different
			randomID2 = random() % empty_blocks;

		*x2 = (empty_ID[randomID2]).x;
		*y2 = (empty_ID[randomID2]).y;
		board[*x1][*y1] = PACMAN;
		board[*x2][*y2] = MONSTER;
		free(empty_ID);
	}
}
