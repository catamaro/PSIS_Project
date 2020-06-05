#include "structs.h"
#include "server.h"
#include "comm.h"
#include "list_handler.h"

pthread_mutex_t mutex_insert_player;

int send_board_dim(int x, int y, int sock_fd){
    char message[50];
    int err;

	memset(message, 0, 50*sizeof(char)); 

    err = sprintf(message, "%d %d\n", x, y);
	if(err == -1){
		printf("error: cannot create message\n");
		return -1;
	}

   	err = write(sock_fd, message, strlen(message)); 
	if(err <= 0){
		perror("write: ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

	printf("\nsvr snd board size: %s\n", message);

    return 0;
}

int send_board_setup(int sock_fd){
	struct pos_list *head = getBrickList();
	struct pos_list *current = head;
	struct player *headPlayer = getPlayerList();
	struct player *currentPlayer = headPlayer;
    int err;

	for (current = head; current != NULL; current = current->next)
	{
		err = send_init_msg(sock_fd, current->character, current->x, current->y, NULL);
		if(err == -1) return -1;
	}

	head = getFruitList();
	current = head;
	
	for (current = head; current != NULL; current = current->next)
	{
		err = send_init_msg(sock_fd, current->character, current->x, current->y, NULL);
		if(err == -1) return -1;
	}
	
	// end of messages of type 1
	err = send_init_msg(sock_fd, -1, -1, -1, NULL);
	if(err == -1) return -1;

	for (currentPlayer = headPlayer; currentPlayer != NULL; currentPlayer = currentPlayer->next)
	{

		err = send_init_msg(sock_fd, MONSTER, currentPlayer->monster->x, currentPlayer->monster->y, currentPlayer->rgb);
		if(err == -1) return -1;

		if(currentPlayer->times == 0)
			err = send_init_msg(sock_fd, PACMAN, currentPlayer->pacman->x, currentPlayer->pacman->y, currentPlayer->rgb);
		else
			err = send_init_msg(sock_fd, SUPERPACMAN, currentPlayer->pacman->x, currentPlayer->pacman->y, currentPlayer->rgb);
		
		if(err == -1) return -1;
	}
	
	// end of messages of type 2
	err = send_init_msg(sock_fd, -1, -1, -1, NULL);
	if(err == -1) return -1;

	return 0;
}

int send_init_msg(int sock_fd, int type, int x, int y, struct color *rgb){
	int err;

	if (type == PACMAN || type == MONSTER || type == SUPERPACMAN){
		struct init_msg_2 *message = malloc(sizeof(struct init_msg_2));
		message->r = rgb->r;
		message->g = rgb->g;
		message->b = rgb->b;
		message->character = type;
		message->x = x;
		message->y = y;
		
		err = write(sock_fd, message, sizeof(*message)); 
		if(err <= 0){
			perror("write: ");
			close(sock_fd);
			exit(EXIT_FAILURE);
		}
		printf("\nsvr snd initial positions: %d %d %d %d %d\n", message->x, message->y, 
					message->r, message->g, message->b);
	}
	else{
		struct init_msg_1 *message = malloc(sizeof(struct init_msg_1));
		message->character = type;
		message->x = x;
		message->y = y;

		err = write(sock_fd, message, sizeof(*message)); 
		if(err <= 0){
			perror("write: ");
			close(sock_fd);
			exit(EXIT_FAILURE);
		}
		printf("\nsvr snd initial positions: %d %d\n", message->x, message->y);
	}

	
	return 0;
}

int send_position(struct position *pacman, struct position *monster, int sock_fd){
	char message[50];
    int err;

	memset(message, 0, 50*sizeof(char)); 

    err = sprintf(message, "%d %d %d %d\n", pacman->x, pacman->y, monster->x, monster->y);
	if(err == -1){
		printf("error: cannot create message\n");
		return -1;
	}

   	err = write(sock_fd, message, strlen(message)); 
	if(err <= 0){
		perror("write: ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

	printf("\nsvr snd initial positions: %s\n", message);

    return 0;
}

int rcv_color(int sock_fd, color *new_color){
    int err;

	err = recv(sock_fd, new_color , sizeof(*new_color), 0);	
	if(err <= 0){
		perror("receive ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	} 

	printf("\nsvr rcv color: %d %d %d\n", new_color->r, new_color->g, new_color->b);
	
	return 0;	
}

int rcv_event(int sock_fd, SDL_Event *new_event, int *type){
	int err;
	int new_x, new_y, dir;
	struct player *list;
	struct position *new_position = malloc(sizeof(struct position));
	struct init_msg_1 *message = malloc(sizeof(struct init_msg_1));

	// falta usar mutex para impedir que várias threads corram ao mesmo tempo

	err = recv(sock_fd, message , sizeof(*message), 0);
	if(err <= 0){
		perror("receive ");
		return -1;
	}

	list = getPlayerList();
	if (list == NULL) return -1; // there are no players

	while(list != NULL){
		if(sock_fd == list->sock_fd) break;
		list = list->next;
	}

	if(message->character == PACMAN){
		*type = PACMAN;

		printf("svr rcv event: %d %d %d\n", message->character, message->x, message->y);

		// store new position in motion 
		new_position->x = message->x;
		new_position->y = message->y;
		// store previous position in user data
		new_event->user.data1 = list;
		new_event->user.data2 = new_position;
	}
	else if(message->character == MONSTER){
		*type = MONSTER;

		printf("svr rcv event: %d %d\n", message->character, message->x);

		new_x = list->monster->x;
		new_y = list->monster->y;
		dir = message->x;
		
		switch (dir)
		{
			case LEFT:  new_x--; break;
			case RIGHT: new_x++; break;
			case UP: 	new_y--; break;
			case DOWN: 	new_y++; break;
		}
		new_position->x = new_x;
		new_position->y = new_y;

		// store previous position in user data
		new_event->user.data1 = list;
		new_event->user.data2 = new_position;
	}

	return 0;
}

int broadcast_update(int x_new, int y_new, int x, int y, int character, struct color *rgb){
	struct player *head = getPlayerList();
	struct player *current = head;
	int err;

	for (current = head; current != NULL; current = current->next)
	{
		printf("id: %d type:%d x:%d y:%d\n", current->id, character, x, y);
		err = send_update(current->sock_fd, character, x, y, x_new, y_new, rgb);
		if(err == -1) return -1;
	}

	return 0;
}

int send_update(int sock_fd, int type, int x, int y, int new_x, int new_y, struct color *rgb){
	struct update_msg *message = malloc(sizeof(struct update_msg));
	int err;

	message->character = type;
	message->new_x = new_x;
	message->new_y = new_y;
	message->x = x;
	message->y = y;
	if(rgb != NULL){
		message->r = rgb->r;
		message->g = rgb->g;
		message->b = rgb->b;
	}
	else{
		message->r = -1;
		message->g = -1;
		message->b = -1;
	}
	
	err = write(sock_fd, message, sizeof(*message)); 
	if(err <= 0){
		perror("write: ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

	return 0;
}

int broadcast_score(int player_id, int score){
	struct player *head = getPlayerList();
	struct player *current = head;
	int err;
	
	for (current = head; current != NULL; current = current->next)
	{
		err = send_score(current->sock_fd, player_id, score);
		if(err == -1) return -1;
	}

	return 0;
}

int send_score(int sock_fd, int player_id, int score){
	struct init_msg_1 *message = malloc(sizeof(struct init_msg_1));
	int err;
	
	message->x = player_id;
	message->y = score;
	message->character = SCORE;
	if(pthread_mutex_trylock(&mutex_insert_player) == 0){
		err = write(sock_fd, message, sizeof(*message)); 
		if(err <= 0){
			perror("write: ");
			close(sock_fd);
			exit(EXIT_FAILURE);
		}
		pthread_mutex_unlock(&mutex_insert_player);	
	}

	return 0;
}

void accept_client(int board_x, int board_y, struct position *pacman, struct position *monster, 
					struct color *new_color, int *num_players, int new_fd){
	struct player *new_player;

	int err;

	pthread_mutex_lock(&mutex_insert_player);

	// add new player to player list
	new_player = insertPlayer(pacman, monster, new_color, *num_players, new_fd);

	// creeate thread for new client
	pthread_create(&(new_player->thread_id), NULL, threadClient, (void *)&(new_player->sock_fd));

	// send board dimensions to new client
	err = send_board_dim(board_x, board_y, new_player->sock_fd);
	sleep(1);
	if (err == -1)
		exit(EXIT_FAILURE);

	err = send_board_setup(new_player->sock_fd);
	if (err == -1)
		exit(EXIT_FAILURE);

	printf("\nPlayer %d entered the game\n", *num_players);	

	pthread_mutex_unlock(&mutex_insert_player);
}

void init_insert_player_mutex(){
	pthread_mutex_init(&mutex_insert_player, NULL);
}

void destroy_insert_player_mutex(){
	pthread_mutex_destroy(&mutex_insert_player);
}
	
int** CheckInactivity(int **board)
{
	//start from the first link
	struct player *current = getPlayerList();

	if (current == NULL)
		return board;

	//navigate through list
	while (current)
	{		
		if(pthread_mutex_trylock(&mutex_insert_player) == 0){
			//if it is last player
			if (current->next == NULL)
			{

				if (current->inactive_time_pacman >= (1000 * 30))
				{
					// Pacman Inativo
					int x, y, x_old, y_old;
					RandomPositionRules(&x, &y);
					x_old = current->pacman->x;
					y_old = current->pacman->y;
					board[x_old][y_old] = EMPTY;
					clear_place(x_old, y_old);
					current->pacman->x = x;
					current->pacman->y = y;

					if (current->times < 1)
					{
						board[x][y] = PACMAN;
						paint_pacman(x, y, current->rgb->r, current->rgb->g, current->rgb->b);
						broadcast_update(x, y, x_old, y_old, PACMAN, current->rgb);
						
					}
					else
					{
						board[x][y] = SUPERPACMAN;
						paint_powerpacman(x, y, current->rgb->r, current->rgb->g, current->rgb->b);
						broadcast_update(x, y, x_old, y_old, SUPERPACMAN, current->rgb);
					}
					current->inactive_time_pacman = 0;
				}
				else
				{
					current->inactive_time_pacman = current->inactive_time_pacman + 1000;
				}
				if (current->inactive_time_monster >= (1000 * 30))
				{
					// Monstro Inativo
					int x, y, x_old, y_old;
					RandomPositionRules(&x, &y);
					x_old = current->monster->x;
					y_old = current->monster->y;
					board[x_old][y_old] = EMPTY;
					clear_place(x_old, y_old);
					current->monster->x = x;
					current->monster->y = y;
					board[x][y] = MONSTER;
					paint_monster(x, y, current->rgb->r, current->rgb->g, current->rgb->b);
					broadcast_update(x, y, x_old, y_old, MONSTER, current->rgb);
					current->inactive_time_monster = 0;
				}
				else
				{
					current->inactive_time_monster = current->inactive_time_monster + 1000;
				}
				current = current->next;
				break;
			}
			else
			{
				if (current->inactive_time_pacman >= (1000 * 3))
				{
					// Pacman Inativo
					int x, y, x_old, y_old;
					RandomPositionRules(&x, &y);
					x_old = current->pacman->x;
					y_old = current->pacman->y;
					board[x_old][y_old] = EMPTY;
					clear_place(x_old, y_old);
					current->pacman->x = x;
					current->pacman->y = y;
					if (current->times < 1)
					{
						board[x][y] = PACMAN;
						paint_pacman(x, y, current->rgb->r, current->rgb->g, current->rgb->b);
						broadcast_update(x, y, x_old, y_old, PACMAN, current->rgb);
					}
					else
					{
						board[x][y] = SUPERPACMAN;
						paint_powerpacman(x, y, current->rgb->r, current->rgb->g, current->rgb->b);
						broadcast_update(x, y, x_old, y_old, SUPERPACMAN, current->rgb);
					}
					current->inactive_time_pacman = 0;
				}
				else
				{
					current->inactive_time_pacman = current->inactive_time_pacman + 1000;
				}
				if (current->inactive_time_monster >= (1000 * 3))
				{
					// Monstro Inativo
					int x, y, x_old, y_old;
					RandomPositionRules(&x, &y);
					x_old = current->monster->x;
					y_old = current->monster->y;
					board[x_old][y_old] = EMPTY;
					clear_place(x_old, y_old);
					current->monster->x = x;
					current->monster->y = y;
					board[x][y] = MONSTER;
					paint_monster(x, y, current->rgb->r, current->rgb->g, current->rgb->b);
					broadcast_update(x, y, x_old, y_old, MONSTER, current->rgb);
					current->inactive_time_monster = 0;
				}
				else
				{
					current->inactive_time_monster = current->inactive_time_monster + 1000;
				}
				current = current->next;
			}
		
			pthread_mutex_unlock(&mutex_insert_player);
			printf("I live the mutex!\n");
		}
		pthread_mutex_unlock(&mutex_insert_player);
		printf("I live the mutex!\n");
	}

	return board;
}

void ManageFruits(int *num_fruits, int *num_players, int ***board)
{

	if (*num_players <= 1)
		return;
	if (*num_fruits == (*num_players - 1) * 2)
		return;
	if (*num_fruits > (*num_players - 1) * 2)
	{
		do
		{
			int x, y;
			FetchFruitHeadCoords(&x, &y);
			RemoveFruitHead();

			clear_place(x, y);
			(*board)[x][y] = 0;
			(*num_fruits)--;
			broadcast_update(x, y, x, y, (*board)[x][y], NULL);
			
		} while (*num_fruits > (*num_players - 1) * 2);

		pthread_mutex_unlock(&mutex_insert_player);
		printf("I live mutex\n");

		return;
	}
	do
	{
		int x, y;
		RandomPositionRules(&x, &y);

		int fruit = rand() % (CHERRY + 1 - LEMON) + LEMON;
		AddPosHead(x, y, fruit);

		(*num_fruits)++;
		if (fruit == LEMON)
		{
			(*board)[x][y] = LEMON;
			paint_lemon(x, y);
		}
		else if (fruit == CHERRY)
		{
			(*board)[x][y] = CHERRY;
			paint_cherry(x, y);
		}
		broadcast_update(x, y, x, y, (*board)[x][y], NULL);

	} while (*num_fruits < (*num_players - 1) * 2);		

}