#include "structs.h"
#include "server.h"
#include "list_handler.h"
#include "comm.h"
#include "game_rules.h"

int server_fd;
int board_x;
int board_y;
int empty_blocks = 0;
int num_fruits = 0;
int num_players = 0;
int num_bricks = 0;
int **board;
int done = 0;

unsigned int fruit_update_time;
unsigned int score_update_time;
unsigned int Event_MovePacman = 0;
unsigned int Event_MoveMonster = 1;

#define MAX_PLAYERS (int)floor((board_x * board_y - num_bricks + 2) / 4)

int main(int argc, char *argv[])
{
	srand(time(NULL));
	SDL_Event event;
	struct sockaddr_in local_addr;
	int port = 58001;

	if (argc != 2)
	{
		printf("error: wrong arguments\n");
		exit(EXIT_FAILURE);
	}
	getValues(&fruit_update_time, &num_bricks, &num_players, &num_fruits, &empty_blocks, &board);
	board = loadBoard(argv[1], &board_x, &board_y);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		perror("socket: ");
		exit(EXIT_FAILURE);
	}

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = htons(port);
	int err = bind(server_fd, (struct sockaddr *)&local_addr,
				   sizeof(local_addr));
	if (err == -1)
	{
		perror("bind: ");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, MAX_PLAYERS_WAITING) == -1)
	{
		perror("listen: ");
		exit(EXIT_FAILURE);
	}

	//accept thread id
	pthread_t thread_id;

	//accepts new player connections
	err = pthread_create(&thread_id, NULL, threadAccept, NULL);
	if(err != 0){
		printf("error: could not create thread\n");
		exit(EXIT_FAILURE);
	}

	printf("Use port %d to access server\n", port);

	// initialization of the mutex
	init_insert_player_mutex();
	init_run_rcv_event();
	init_run_snd_event();

	// create sig action for ctrl+c
	struct sigaction sa;
    sa.sa_handler = sigHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);

	int x_new, y_new, x, y;
	int x_new1 = -1, x_new2 = -1, y_new1 = -1, y_new2 = -1;
	int x_old1 = -1, x_old2 = -1, y_old1 = -1, y_old2 = -1;
	struct player *player1, *player2;
	struct position *new_position;

	fruit_update_time = SDL_GetTicks();
	score_update_time = SDL_GetTicks();

	// main loop that processes the events
	while (!done)
	{
		SDL_Delay(20);
		unsigned int current_time = SDL_GetTicks();
		unsigned int delta_score = current_time - score_update_time;
		unsigned int delta_fruit = current_time - fruit_update_time;

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

				if(event.type == Event_MoveMonster && player1->monster_tokens == 0){
					continue;
				}
				if(event.type == Event_MovePacman && player1->pacman_tokens == 0)
				{
					continue;
				}

				if (event.type == Event_MoveMonster)
				{
					player1->inactive_time_monster = 0;
					player1->monster_tokens = player1->monster_tokens - 1;
					// Test for bricks and out of bound
					bounceBounds(x, y, &x_new, &y_new);

					player2 = findPlayerPos(x_new, y_new, MONSTER);
					if (player2 == NULL)
						player2 = findPlayerPos(x_new, y_new, PACMAN);

					x_new1 = -1, x_new2 = -1, y_new1 = -1, y_new2 = -1;
					x_old1 = -1, x_old2 = -1, y_old1 = -1, y_old2 = -1;
					checkRulesMonster(player1, player2, x_new, y_new, &x_new1, &y_new1, &x_new2, &y_new2,
									  &x_old1, &y_old1, &x_old2, &y_old2);
					broadcast_update(x_new1, y_new1, x_old1, y_old1, board[x_new1][y_new1], player1->rgb);

					if (x_new2 != -1)
					{
						if (player2 != NULL)
							broadcast_update(x_new2, y_new2, x_old2, y_old2, board[x_new2][y_new2], player2->rgb);
						else
							broadcast_update(x_new2, y_new2, x_old2, y_old2, board[x_new2][y_new2], NULL);
					}
				}
				else
				{
					player1->inactive_time_pacman = 0;
					player1->pacman_tokens = player1->pacman_tokens - 1;
					// Test for bricks and out of bound
					bounceBounds(x, y, &x_new, &y_new);

					player2 = findPlayerPos(x_new, y_new, PACMAN);
					if (player2 == NULL)
						player2 = findPlayerPos(x_new, y_new, MONSTER);

					if (board[x][y] == PACMAN)
					{
						x_new1 = -1, x_new2 = -1, y_new1 = -1, y_new2 = -1;
						x_old1 = -1, x_old2 = -1, y_old1 = -1, y_old2 = -1;
						checkRulesPacman(player1, player2, x_new, y_new, &x_new1, &y_new1, &x_new2, &y_new2,
										 &x_old1, &y_old1, &x_old2, &y_old2);
						broadcast_update(x_new1, y_new1, x_old1, y_old1, board[x_new1][y_new1], player1->rgb);

						if (x_new2 != -1)
						{
							if (player2 != NULL)
								broadcast_update(x_new2, y_new2, x_old2, y_old2, board[x_new2][y_new2], player2->rgb);
							else
								broadcast_update(x_new2, y_new2, x_old2, y_old2, board[x_new2][y_new2], NULL);
						}
					}
					else if (board[x][y] == SUPERPACMAN)
					{
						x_new1 = -1, x_new2 = -1, y_new1 = -1, y_new2 = -1;
						x_old1 = -1, x_old2 = -1, y_old1 = -1, y_old2 = -1;
						checkRulesSuperPacman(player1, player2, x_new, y_new, &x_new1, &y_new1, &x_new2, &y_new2,
											  &x_old1, &y_old1, &x_old2, &y_old2);
						broadcast_update(x_new1, y_new1, x_old1, y_old1, board[x_new1][y_new1], player1->rgb);

						if (x_new2 != -1)
						{
							if (player2 != NULL)
								broadcast_update(x_new2, y_new2, x_old2, y_old2, board[x_new2][y_new2], player2->rgb);
							else
								broadcast_update(x_new2, y_new2, x_old2, y_old2, board[x_new2][y_new2], NULL);
						}
					}
				}
			}
		}
		// Poll a cada 2 segundos para adicionar nova fruta
		if (delta_fruit >= 1000 * 2)
		{
			ManageFruits();
			fruit_update_time = current_time;
		}
		// Poll a cada 1 minuto para dar print ao score
		if (delta_score >= (1000 * 60) && num_players > 1)
		{
			PrintPlayerScore();
			score_update_time = current_time;
		}
	}

	free(board[0]);
	free(board);
	freeList();
	freePosList();

	printf("fim\n");
	close_board_windows();
	close(server_fd);

	destroy_insert_player_mutex();
	exit(EXIT_SUCCESS);
}

void *threadAccept(void *arg)
{
	struct sockaddr_in client_addr;
	socklen_t size_addr = sizeof(client_addr);
	int new_fd;
	size_t err;
	
	while (!done)
	{
		struct color *new_color = malloc(sizeof(struct color));
		struct position *pacman = malloc(sizeof(struct position));
		struct position *monster = malloc(sizeof(struct position));

		printf("waiting for players\n");
		new_fd = accept(server_fd,
						(struct sockaddr *)&client_addr, &size_addr);
		if (new_fd == -1)
		{
			perror("accept ");
			exit(EXIT_FAILURE);
		}

		if (num_players + 1 > MAX_PLAYERS)
		{
			printf("maximum number of players achieved\n");
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
		RandomPositionConnect(&(pacman->x), &(monster->x), &(pacman->y), &(monster->y));
		// update board with position
		board[pacman->x][pacman->y] = PACMAN;
		board[monster->x][monster->y] = MONSTER;
		// paint both characters in server board
		paint_pacman(pacman->x, pacman->y, new_color->r, new_color->g, new_color->b);
		paint_monster(monster->x, monster->y, new_color->r, new_color->g, new_color->b);

		if(num_players == 1) score_update_time = SDL_GetTicks();

		num_players ++;

		ManageFruits();

		accept_client(board_x, board_y, pacman, monster, new_color, &num_players, 
						new_fd);
						
	}
	

	return (NULL);
}

void *threadClient(void *arg)
{
	struct player *my_player = (struct player *)arg;
	int sock_fd = my_player->sock_fd;
	int err, character;
	SDL_Event new_event;

	while (!done)
	{
		SDL_zero(new_event);

		err = rcv_event(sock_fd, &new_event, &character);
		if (err == -1)
		{
			clientDisconnect(my_player->id, &num_players, &board);

			return(NULL);
		}
		if (character == MONSTER) new_event.type = Event_MoveMonster;
		
		if (character == PACMAN) new_event.type = Event_MovePacman;
		SDL_PushEvent(&new_event);
	}

	return (NULL);
}

void *threadClientTime(void *arg)
{
	struct player *my_player = (struct player *)arg;
	unsigned int inactivity_update_time = SDL_GetTicks();
	unsigned int movement_update_time = SDL_GetTicks();

	while (!done)
	{
		sleep(0.5);

		unsigned int current_time = SDL_GetTicks();
		unsigned int delta_movement = current_time - movement_update_time;
		unsigned int delta_inactivity = current_time - inactivity_update_time;

		if (delta_inactivity >= 1000)
		{
			board = CheckInactivity(board, my_player);
			inactivity_update_time = current_time;
		}
		if(delta_movement >= 500){
			IncrementMovement(my_player);
			movement_update_time = current_time;
		}
	}

	return (NULL);
}


void PrintPlayerScore()
{
	//start from the first link
	struct player *current = getPlayerList();

	if (current == NULL)
		return;

	//navigate through list
	while (current)
	{
		//if it is last player
		if (current->next == NULL)
		{
			printf("Player %d: %d points\n", current->id, current->score);
			broadcast_score(current->id, current->score);
			return;
		}
		else
		{
			printf("Player %d: %d points\n", current->id, current->score);
			broadcast_score(current->id, current->score);
			//go to next link
			current = current->next;
		}
	}
}

void ResetScore()
{
	struct player *playerHead = getPlayerList();
	if (playerHead == NULL)
		return;
	if (playerHead->next != NULL)
		return;
	playerHead->score = 0;
}

void freeBoard(){
	free(board[0]);
	free(board);
}

void closeFd(){
	close(server_fd);
}