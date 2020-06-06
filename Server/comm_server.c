#include "structs.h"
#include "server.h"
#include "comm.h"
#include "list_handler.h"
#include "game_rules.h"

pthread_mutex_t run_insert_player = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t run_rcv_event = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t run_snd_event = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t run_cond = PTHREAD_COND_INITIALIZER;
int run_thread = 0;
int run_thread2 = 0;

void sigHandler(int sig){

	printf("\nctrl+c was pressed application will terminate\n");
	freeBoard();
	freeList();
	freePosList();

	close_board_windows();
	closeFd();

	destroy_insert_player_mutex();
	destroy_run_rcv_event();
	destroy_run_snd_event();

	exit(EXIT_FAILURE);
}

int send_board_dim(int x, int y, int sock_fd){
    struct position *board_dim = malloc(sizeof(struct position));
    int err;

	board_dim->x = x;
	board_dim->y = y;

   	err = write(sock_fd, board_dim, sizeof(*board_dim)); 
	if(err <= 0){
		perror("write: ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

    return 0;
}

int send_board_setup(struct player *new_player){
	struct pos_list *head = getBrickList();
	struct pos_list *current = head;
	struct player *headPlayer = getPlayerList();
	struct player *currentPlayer = headPlayer;
    int err;

	for (current = head; current != NULL; current = current->next)
	{
		err = send_init_msg(new_player->sock_fd, current->character, current->x, current->y, NULL);
		if(err == -1) return -1;
	}

	head = getFruitList();
	current = head;

	for (current = head; current != NULL; current = current->next)
	{
		err = send_init_msg(new_player->sock_fd, current->character, current->x, current->y, NULL);
		if(err == -1) return -1;
	}

	// end of messages of type 1
	err = send_init_msg(new_player->sock_fd, -1, -1, -1, NULL);
	if(err == -1) return -1;

	for (currentPlayer = headPlayer; currentPlayer != NULL; currentPlayer = currentPlayer->next)
	{
		if(new_player != currentPlayer){
			err = send_init_msg(currentPlayer->sock_fd, MONSTER, new_player->monster->x, new_player->monster->y, new_player->rgb);
			if(err == -1) return -1;

			if(new_player->times == 0)
				err = send_init_msg(new_player->sock_fd, PACMAN, currentPlayer->monster->x, currentPlayer->monster->y, currentPlayer->rgb);
			else
				err = send_init_msg(new_player->sock_fd, SUPERPACMAN, currentPlayer->monster->x, currentPlayer->monster->y, currentPlayer->rgb);
		} 
		

		err = send_init_msg(new_player->sock_fd, MONSTER, currentPlayer->monster->x, currentPlayer->monster->y, currentPlayer->rgb);
		if(err == -1) return -1;

		if(currentPlayer->times == 0)
			err = send_init_msg(new_player->sock_fd, PACMAN, currentPlayer->pacman->x, currentPlayer->pacman->y, currentPlayer->rgb);
		else
			err = send_init_msg(new_player->sock_fd, SUPERPACMAN, currentPlayer->pacman->x, currentPlayer->pacman->y, currentPlayer->rgb);

		if(err == -1) return -1;
	}

	// end of messages of type 2
	err = send_init_msg(new_player->sock_fd, -1, -1, -1, NULL);
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
		message->new_x = x;
		message->new_y = y;

		err = write(sock_fd, message, sizeof(*message));
		if(err <= 0){
			perror("write: ");
			close(sock_fd);
			exit(EXIT_FAILURE);
		}
	}
	else{
		struct init_msg_1 *message = malloc(sizeof(struct init_msg_1));
		message->character = type;
		message->new_x = x;
		message->new_y = y;

		err = write(sock_fd, message, sizeof(*message));
		if(err <= 0){
			perror("write: ");
			close(sock_fd);
			exit(EXIT_FAILURE);
		}
	}


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

	return 0;
}

int rcv_event(int sock_fd, SDL_Event *new_event, int *type){
	int err;
	int new_x, new_y, dir;
	struct player *list;
	struct position *new_position = malloc(sizeof(struct position));
	struct init_msg_1 *message = malloc(sizeof(struct init_msg_1));

	//while(!run_thread2);

	pthread_mutex_lock(&run_rcv_event);

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

		// store new position in motion
		new_position->x = message->new_x;
		new_position->y = message->new_y;
		// store previous position in user data
		new_event->user.data1 = list;
		new_event->user.data2 = new_position;
	}
	else if(message->character == MONSTER){
		*type = MONSTER;

		new_x = list->monster->x;
		new_y = list->monster->y;
		dir = message->new_x;

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
	pthread_mutex_unlock(&run_rcv_event);

	return 0;
}

int broadcast_update(int x_new, int y_new, int x, int y, int character, struct color *rgb){
	struct player *head = getPlayerList();
	struct player *current = head;
	int err;

	pthread_mutex_lock(&run_snd_event);

	for (current = head; current != NULL; current = current->next)
	{
		err = send_update(current->sock_fd, character, x, y, x_new, y_new, rgb);
		if(err == -1) return -1;
	}
	pthread_mutex_unlock(&run_snd_event);

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

	pthread_mutex_lock(&run_snd_event);

	for (current = head; current != NULL; current = current->next)
	{
		err = send_score(current->sock_fd, player_id, score);
		if(err == -1) return -1;
	}

	pthread_mutex_unlock(&run_snd_event);

	return 0;
}

int send_score(int sock_fd, int player_id, int score){
	struct init_msg_1 *message = malloc(sizeof(struct init_msg_1));
	int err;

	message->new_x = player_id;
	message->new_y = score;
	message->character = SCORE;

	err = write(sock_fd, message, sizeof(*message));
	if(err <= 0){
		perror("write: ");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}
	sleep(0.5);
	return 0;
}

void accept_client(int board_x, int board_y, struct position *pacman, struct position *monster,
					struct color *new_color, int *num_players, int new_fd){
	struct player *new_player;

	int err;

	run_thread = 0;
	// locks the thread for other threads
	pthread_mutex_lock(&run_insert_player);

	// add new player to player list
	new_player = insertPlayer(pacman, monster, new_color, *num_players, new_fd);

	// creeate thread for new client
	pthread_create(&(new_player->thread_id), NULL, threadClient, (void *)new_player);
	pthread_create(&(new_player->time_id), NULL, threadClientTime, (void *)new_player);

	// send board dimensions to new client
	err = send_board_dim(board_x, board_y, new_player->sock_fd);
	if (err == -1)
		exit(EXIT_FAILURE);

	err = send_board_setup(new_player);
	if (err == -1)
		exit(EXIT_FAILURE);

	printf("\nPlayer %d entered the game\n", *num_players);


    //pthread_cond_signal(&run_cond);
	pthread_mutex_unlock(&run_insert_player);
	run_thread = 1;

}

int** CheckInactivity(int **board, struct player *my_player)
{
	pthread_mutex_trylock(&run_rcv_event);
	
	if ((my_player->inactive_time_pacman) >= (1000 * 30))
	{
		// Pacman Inativo
		int x, y, x_old, y_old;
		RandomPositionRules(&x, &y);
		x_old = my_player->pacman->x;
		y_old = my_player->pacman->y;
		board[x_old][y_old] = EMPTY;
		clear_place(x_old, y_old);
		my_player->pacman->x = x;
		my_player->pacman->y = y;

		if (my_player->times < 1)
		{
			board[x][y] = PACMAN;
			paint_pacman(x, y, my_player->rgb->r, my_player->rgb->g, my_player->rgb->b);
			broadcast_update(x, y, x_old, y_old, PACMAN, my_player->rgb);

		}
		else
		{
			board[x][y] = SUPERPACMAN;
			paint_powerpacman(x, y, my_player->rgb->r, my_player->rgb->g, my_player->rgb->b);
			broadcast_update(x, y, x_old, y_old, SUPERPACMAN, my_player->rgb);
		}
		my_player->inactive_time_pacman = 0;
	}
	else
	{
		my_player->inactive_time_pacman = my_player->inactive_time_pacman + 1000;
	}

	if ((my_player->inactive_time_monster) >= (1000 * 30))
	{
		// monster Inativo
		int x, y, x_old, y_old;
		RandomPositionRules(&x, &y);
		x_old = my_player->monster->x;
		y_old = my_player->monster->y;
		board[x_old][y_old] = EMPTY;
		clear_place(x_old, y_old);
		my_player->monster->x = x;
		my_player->monster->y = y;

		board[x][y] = MONSTER;
		paint_monster(x, y, my_player->rgb->r, my_player->rgb->g, my_player->rgb->b);
		broadcast_update(x, y, x_old, y_old, MONSTER, my_player->rgb);

		my_player->inactive_time_monster = 0;
	}
	else
	{
		my_player->inactive_time_monster = my_player->inactive_time_monster + 1000;
	}

	pthread_mutex_unlock(&run_rcv_event);

	return board;
}

void init_insert_player_mutex(){
	pthread_mutex_init(&run_insert_player, NULL);
}

void destroy_insert_player_mutex(){
	pthread_mutex_destroy(&run_insert_player);
}

void init_run_rcv_event(){
	pthread_mutex_init(&run_rcv_event, NULL);
}

void destroy_run_rcv_event(){
	pthread_mutex_destroy(&run_rcv_event);
}

void init_run_snd_event(){
	pthread_mutex_init(&run_snd_event, NULL);
}

void destroy_run_snd_event(){
	pthread_mutex_destroy(&run_snd_event);
}