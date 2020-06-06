#include "structs.h"
#include "server.h"
#include "list_handler.h"
#include "comm.h"
#include "game_rules.h"

int *bricks;
int *fruits;
int *players;
int *blocks;
int board_x;
int board_y;
int ***main_board;
unsigned int *update_time;

void getValues(unsigned int *fruit_update_time, int *num_bricks, int *num_players, int *num_fruits, 
				int *empty_blocks, int ***board){
	players = num_players;
	fruits = num_fruits;
	blocks = empty_blocks;
	bricks = num_bricks;
	update_time = fruit_update_time;
	main_board = board;
}


void checkRulesMonster(struct player *dealer, struct player *receiver, int x_new, int y_new,
					   int *x_new1, int *y_new1, int *x_new2, int *y_new2,
					   int *x_old1, int *y_old1, int *x_old2, int *y_old2)
{ // Monster moving to empty place
	if ((*main_board)[x_new][y_new] == EMPTY)
	{
		int old_x = dealer->monster->x;
		int old_y = dealer->monster->y;
		*x_old1 = old_x;
		*y_old1 = old_y;
		(*main_board)[old_x][old_y] = EMPTY;
		clear_place(old_x, old_y);
		dealer->monster->x = x_new;
		dealer->monster->y = y_new;
		*x_new1 = x_new;
		*y_new1 = y_new;
		(*main_board)[x_new][y_new] = MONSTER;
		paint_monster(x_new, y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
	}
	// Monster moving to fruit
	else if ((*main_board)[x_new][y_new] == LEMON || (*main_board)[x_new][y_new] == CHERRY)
	{
		int old_x2 = dealer->monster->x;
		int old_y2 = dealer->monster->y;
		*x_old1 = old_x2;
		*y_old1 = old_y2;
		dealer->score = dealer->score + 1;
		(*fruits)--;
		RemoveFruitPosition(x_new, y_new);
		(*main_board)[old_x2][old_y2] = EMPTY;
		clear_place(old_x2, old_y2);
		clear_place(x_new, y_new);
		dealer->monster->x = x_new;
		dealer->monster->y = y_new;
		*x_new1 = x_new;
		*y_new1 = y_new;
		(*main_board)[x_new][y_new] = MONSTER;
		paint_monster(x_new, y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		*update_time = SDL_GetTicks();
	}
	// Monster moving to pacman
	else if ((*main_board)[x_new][y_new] == PACMAN)
	{
		// Same person's pacman
		if (dealer->id == receiver->id)
		{
			// Save values in integers to avoid deffering many pointers
			int old_x1 = receiver->pacman->x;
			int old_y1 = receiver->pacman->y;
			int old_x2 = dealer->monster->x;
			int old_y2 = dealer->monster->y;

			*x_old1 = old_x2;
			*y_old1 = old_y2;
			*x_old2 = -1;
			*y_old2 = old_y1;

			clear_place(old_x1, old_y1);
			clear_place(old_x2, old_y2);

			// Put monster in pacman's block
			dealer->monster->x = old_x1;
			dealer->monster->y = old_y1;
			// Put pacman in monster's block
			receiver->pacman->x = old_x2;
			receiver->pacman->y = old_y2;
			(*main_board)[old_x2][old_y2] = PACMAN;
			(*main_board)[old_x1][old_y1] = MONSTER;
			*x_new1 = old_x1;
			*y_new1 = old_y1;
			*x_new2 = old_x2;
			*y_new2 = old_y2;
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
			*x_old1 = old_x2;
			*y_old1 = old_y2;
			*x_old2 = -1;
			*y_old2 = old_y1;
			// Clean old pacman space
			(*main_board)[old_x2][old_y2] = EMPTY;
			clear_place(old_x2, old_y2);
			// Save new monster coordinates, increment score and paint
			dealer->monster->x = old_x1;
			dealer->monster->y = old_y1;
			*x_new1 = old_x1;
			*y_new1 = old_y1;
			dealer->score = dealer->score + 1;
			(*main_board)[old_x1][old_y1] = MONSTER;
			paint_monster(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
			// Fetch new pacman coordinates, save and paint
			RandomPositionRules(&old_x1, &old_y1);
			*x_new2 = old_x1;
			*y_new2 = old_y1;
			(*main_board)[old_x1][old_y1] = PACMAN;
			receiver->pacman->x = old_x1;
			receiver->pacman->y = old_y1;
			paint_pacman(old_x1, old_y1, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
		}
	}
	// Monster moving to superpacman
	else if ((*main_board)[x_new][y_new] == SUPERPACMAN)
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

			*x_old1 = old_x2;
			*y_old1 = old_y2;
			*x_old2 = -1;
			*y_old2 = old_y1;
			// Put monster in superpacman's block
			dealer->monster->x = old_x1;
			dealer->monster->y = old_y1;
			// Put superpacman in monster's block
			receiver->pacman->x = old_x2;
			receiver->pacman->y = old_y2;

			*x_new1 = old_x1;
			*y_new1 = old_y1;
			*x_new2 = old_x2;
			*y_new2 = old_y2;

			(*main_board)[old_x2][old_y2] = SUPERPACMAN;
			(*main_board)[old_x1][old_y1] = MONSTER;

			paint_powerpacman(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
			paint_monster(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		}
		else
		{
			// Another person's superpacman
			int old_x1 = receiver->pacman->x;
			int old_y1 = receiver->pacman->y;
			int old_x2 = dealer->monster->x;
			int old_y2 = dealer->monster->y;
			*x_old1 = old_x2;
			*y_old1 = old_y2;
			*x_old2 = old_x1;
			*y_old2 = old_y1;
			// Increment score for superpacman
			receiver->score = receiver->score + 1;
			// Decrement number of times superpacman can eat
			receiver->times = receiver->times - 1;
			// If superpacman ate 2 monsters, go back to normal pacman
			if (receiver->times < 1)
			{
				clear_place(old_x1, old_y1);
				(*main_board)[old_x1][old_y1] = PACMAN;
				paint_powerpacman(old_x1, old_y1, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
			}
			// Clean old monster coordinates
			(*main_board)[old_x2][old_y2] = EMPTY;
			clear_place(old_x2, old_y2);
			// New monster coordinates
			RandomPositionRules(&old_x2, &old_y2);
			(*main_board)[old_x2][old_y2] = MONSTER;
			dealer->monster->x = old_x2;
			dealer->monster->y = old_y2;
			*x_new1 = old_x2;
			*y_new1 = old_y2;
			*x_new2 = old_x1;
			*y_new2 = old_y1;
			paint_monster(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		}
	}
	// Monster moving to another monster
	else if ((*main_board)[x_new][y_new] == MONSTER)
	{
		int old_x1 = receiver->monster->x;
		int old_y1 = receiver->monster->y;
		int old_x2 = dealer->monster->x;
		int old_y2 = dealer->monster->y;
		clear_place(old_x1, old_y1);
		clear_place(old_x2, old_y2);
		*x_old1 = old_x2;
		*y_old1 = old_y2;
		*x_old2 = -1;
		*y_old2 = old_y1;
		// Swap monsters
		dealer->monster->x = old_x1;
		dealer->monster->y = old_y1;
		receiver->monster->x = old_x2;
		receiver->monster->y = old_y2;
		*x_new1 = old_x1;
		*y_new1 = old_y1;
		*x_new2 = old_x2;
		*y_new2 = old_y2;
		paint_monster(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		paint_monster(old_x2, old_y2, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
	}
}

void checkRulesPacman(struct player *dealer, struct player *receiver, int x_new, int y_new,
					  int *x_new1, int *y_new1, int *x_new2, int *y_new2,
					  int *x_old1, int *y_old1, int *x_old2, int *y_old2)
{
	// Pacman moving to empty place
	if ((*main_board)[x_new][y_new] == EMPTY)
	{
		int old_x = dealer->pacman->x;
		int old_y = dealer->pacman->y;
		(*main_board)[old_x][old_y] = EMPTY;
		clear_place(old_x, old_y);
		*x_old1 = old_x;
		*y_old1 = old_y;
		dealer->pacman->x = x_new;
		dealer->pacman->y = y_new;
		*x_new1 = x_new;
		*y_new1 = y_new;
		(*main_board)[x_new][y_new] = PACMAN;
		paint_pacman(x_new, y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		return;
	}
	// Pacman moving to fruit
	else if ((*main_board)[x_new][y_new] == LEMON || (*main_board)[x_new][y_new] == CHERRY)
	{
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		*x_old1 = old_x2;
		*y_old1 = old_y2;
		dealer->score = dealer->score + 1;
		(*fruits)--;
		RemoveFruitPosition(x_new, y_new);
		(*main_board)[old_x2][old_y2] = EMPTY;
		clear_place(old_x2, old_y2);
		clear_place(x_new, y_new);
		dealer->pacman->x = x_new;
		dealer->pacman->y = y_new;
		*x_new1 = x_new;
		*y_new1 = y_new;
		dealer->times = 2;
		(*main_board)[x_new][y_new] = SUPERPACMAN;
		paint_powerpacman(x_new, y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		*update_time = SDL_GetTicks();
	}
	// Pacman moving to monster
	else if ((*main_board)[x_new][y_new] == MONSTER)
	{
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

			*x_old1 = old_x2;
			*y_old1 = old_y2;
			*x_old2 = -1;
			*y_old2 = old_y1;

			// Put pacman in monster's block
			dealer->pacman->x = old_x1;
			dealer->pacman->y = old_y1;
			// Put monster in pacman's block
			receiver->monster->x = old_x2;
			receiver->monster->y = old_y2;

			(*main_board)[old_x2][old_y2] = MONSTER;
			(*main_board)[old_x1][old_y1] = PACMAN;

			*x_new1 = old_x1;
			*y_new1 = old_y1;
			*x_new2 = old_x2;
			*y_new2 = old_y2;

			paint_monster(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
			paint_pacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		}
		// Another person's monster
		else
		{
			int old_x2 = dealer->pacman->x;
			int old_y2 = dealer->pacman->y;

			*x_old1 = old_x2;
			*y_old1 = old_y2;

			// Increment score for monster
			receiver->score = receiver->score + 1;
			// Clean old pacman coordinates
			(*main_board)[old_x2][old_y2] = EMPTY;
			clear_place(old_x2, old_y2);
			// New pacman coordinates
			RandomPositionRules(&old_x2, &old_y2);
			*x_new1 = old_x2;
			*y_new1 = old_y2;
			(*main_board)[old_x2][old_y2] = PACMAN;
			dealer->pacman->x = old_x2;
			dealer->pacman->y = old_y2;
			paint_pacman(old_x2, old_y2, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		}
	}
	// Pacman moving to superpacman
	else if ((*main_board)[x_new][y_new] == SUPERPACMAN)
	{
		int old_x1 = receiver->pacman->x;
		int old_y1 = receiver->pacman->y;
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		clear_place(old_x1, old_y1);
		clear_place(old_x2, old_y2);
		*x_old1 = old_x2;
		*y_old1 = old_y2;
		*x_old2 = -1;
		*y_old2 = old_y1;
		// Swap pacman and superpacman
		dealer->pacman->x = old_x1;
		dealer->pacman->y = old_y1;
		receiver->pacman->x = old_x2;
		receiver->pacman->y = old_y2;
		*x_new1 = old_x1;
		*y_new1 = old_y1;
		*x_new2 = old_x2;
		*y_new2 = old_y2;
		(*main_board)[old_x1][old_y1] = PACMAN;
		(*main_board)[old_x2][old_y2] = SUPERPACMAN;
		paint_pacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		paint_powerpacman(old_x2, old_y2, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
	}
	// Pacman moving to another pacman
	else if ((*main_board)[x_new][y_new] == PACMAN)
	{
		int old_x1 = receiver->pacman->x;
		int old_y1 = receiver->pacman->y;
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		clear_place(old_x1, old_y1);
		clear_place(old_x2, old_y2);
		*x_old1 = old_x2;
		*y_old1 = old_y2;
		*x_old2 = -1;
		*y_old2 = old_y1;
		// Swap pacmans
		dealer->pacman->x = old_x1;
		dealer->pacman->y = old_y1;
		receiver->pacman->x = old_x2;
		receiver->pacman->y = old_y2;
		*x_new1 = old_x1;
		*y_new1 = old_y1;
		*x_new2 = old_x2;
		*y_new2 = old_y2;
		paint_pacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		paint_pacman(old_x2, old_y2, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
	}
}

void checkRulesSuperPacman(struct player *dealer, struct player *receiver, int x_new, int y_new,
						   int *x_new1, int *y_new1, int *x_new2, int *y_new2,
						   int *x_old1, int *y_old1, int *x_old2, int *y_old2)
{
	// Superpacman moving to empty place
	if ((*main_board)[x_new][y_new] == EMPTY)
	{
		int old_x = dealer->pacman->x;
		int old_y = dealer->pacman->y;
		*x_old1 = old_x;
		*y_old1 = old_y;
		(*main_board)[old_x][old_y] = EMPTY;
		clear_place(old_x, old_y);
		dealer->pacman->x = x_new;
		dealer->pacman->y = y_new;
		*x_new1 = x_new;
		*y_new1 = y_new;
		(*main_board)[x_new][y_new] = SUPERPACMAN;
		paint_powerpacman(x_new, y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
	}
	// Superpacman moving to fruit
	else if ((*main_board)[x_new][y_new] == LEMON || (*main_board)[x_new][y_new] == CHERRY)
	{
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		*x_old1 = old_x2;
		*y_old1 = old_y2;
		dealer->score = dealer->score + 1;
		(*fruits)--;
		RemoveFruitPosition(x_new, y_new);
		(*main_board)[old_x2][old_y2] = EMPTY;
		clear_place(old_x2, old_y2);
		clear_place(x_new, y_new);
		dealer->pacman->x = x_new;
		dealer->pacman->y = y_new;
		*x_new1 = x_new;
		*y_new1 = y_new;
		(*main_board)[x_new][y_new] = SUPERPACMAN;
		paint_powerpacman(x_new, y_new, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		*update_time = SDL_GetTicks();
	}
	// Superpacman moving to monster
	else if ((*main_board)[x_new][y_new] == MONSTER)
	{
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

			*x_old1 = old_x2;
			*y_old1 = old_y2;
			*x_old2 = -1;
			*y_old2 = old_y1;

			// Put superpacman in monster's block
			dealer->pacman->x = old_x1;
			dealer->pacman->y = old_y1;
			// Put monster in superpacman's block
			receiver->monster->x = old_x2;
			receiver->monster->y = old_y2;

			*x_new1 = old_x1;
			*y_new1 = old_y1;
			*x_new2 = old_x2;
			*y_new2 = old_y2;

			(*main_board)[old_x2][old_y2] = MONSTER;
			(*main_board)[old_x1][old_y1] = SUPERPACMAN;

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
			*x_old1 = old_x2;
			*y_old1 = old_y2;
			*x_old2 = -1;
			*y_old2 = old_y1;
			// Increment score for superpacman
			dealer->score = dealer->score + 1;
			// Decrement number of times superpacman can eat
			dealer->times = dealer->times - 1;
			// If superpacman ate 2 monsters, go back to normal pacman
			if (dealer->times < 1)
			{
				(*main_board)[old_x1][old_y1] = PACMAN;
				paint_pacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
}
			else
			{
				(*main_board)[old_x1][old_y1] = SUPERPACMAN;
				paint_powerpacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
			}
			dealer->pacman->x = old_x1;
			dealer->pacman->y = old_y1;
			// Clean old superpacman coordinates
			(*main_board)[old_x2][old_y2] = EMPTY;
			clear_place(old_x2, old_y2);
			// New monster coordinates
			RandomPositionRules(&old_x2, &old_y2);
			(*main_board)[old_x2][old_y2] = MONSTER;
			receiver->monster->x = old_x2;
			receiver->monster->y = old_y2;
			*x_new1 = old_x1;
			*y_new1 = old_y1;
			*x_new2 = old_x2;
			*y_new2 = old_y2;
			paint_monster(old_x2, old_y2, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
		}
	}
	// Superpacman moving to another superpacman
	else if ((*main_board)[x_new][y_new] == SUPERPACMAN)
	{
		int old_x1 = receiver->pacman->x;
		int old_y1 = receiver->pacman->y;
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		clear_place(old_x1, old_y1);
		clear_place(old_x2, old_y2);
		*x_old1 = old_x2;
		*y_old1 = old_y2;
		*x_old2 = -1;
		*y_old2 = old_y1;
		// Swap pacmans
		dealer->pacman->x = old_x1;
		dealer->pacman->y = old_y1;
		receiver->pacman->x = old_x2;
		receiver->pacman->y = old_y2;
		*x_new1 = old_x1;
		*y_new1 = old_y1;
		*x_new2 = old_x2;
		*y_new2 = old_y2;
		paint_powerpacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		paint_powerpacman(old_x2, old_y2, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
	}
	// Superpacman moving to a pacman
	else if ((*main_board)[x_new][y_new] == PACMAN)
	{
		int old_x1 = receiver->pacman->x;
		int old_y1 = receiver->pacman->y;
		int old_x2 = dealer->pacman->x;
		int old_y2 = dealer->pacman->y;
		clear_place(old_x1, old_y1);
		clear_place(old_x2, old_y2);
		*x_old1 = old_x2;
		*y_old1 = old_y2;
		*x_old2 = -1;
		*y_old2 = old_y1;
		// Swap pacmans
		dealer->pacman->x = old_x1;
		dealer->pacman->y = old_y1;
		receiver->pacman->x = old_x2;
		receiver->pacman->y = old_y2;
		*x_new1 = old_x1;
		*y_new1 = old_y1;
		*x_new2 = old_x2;
		*y_new2 = old_y2;
		(*main_board)[old_x1][old_y1] = SUPERPACMAN;
		(*main_board)[old_x2][old_y2] = PACMAN;
		paint_powerpacman(old_x1, old_y1, dealer->rgb->r, dealer->rgb->g, dealer->rgb->b);
		paint_pacman(old_x2, old_y2, receiver->rgb->r, receiver->rgb->g, receiver->rgb->b);
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
	else if ((*main_board)[*x_new][*y_new] == BRICK)
	{
		// If moving from left to block
		if (*x_new > x1 && *y_new == y1)
		{
			// If bounce would make it out of bounds
			if (*x_new - 2 < 0)
				*x_new = 0;
			// If bounce would put on another brick
			else if ((*main_board)[*x_new - 2][*y_new] == BRICK)
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
			else if ((*main_board)[*x_new + 2][*y_new] == BRICK)
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
			else if ((*main_board)[*x_new][*y_new - 2] == BRICK)
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
			else if ((*main_board)[*x_new][*y_new + 2] == BRICK)
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
	// Current number of empty blocks
	(*blocks) = (board_x * board_y) - (*bricks) - ((*players) * 2) - (*fruits);
	int threshold = board_x * board_y * 0.3; // Threshold percentage
	if ((*blocks) > threshold)			 // While there are many empty blocks
	{
		// New coordinates
		*x = random() % board_x;
		*y = random() % board_y;
		while ((*main_board)[*x][*y] != EMPTY) // Guarantee that it only finds an empty block
		{
			*x = random() % board_x;
			*y = random() % board_y;
		}
	}
	else // If there aren't many empty blocks
	{
		struct position *empty_ID = malloc((*blocks) * sizeof(position));
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
				if ((*main_board)[i][j] == EMPTY && k < (*blocks))
				{
					(empty_ID[k]).x = i;
					(empty_ID[k]).y = j;
					k++;
				}
			}
		}
		// New coordinates
		randomID = (int)random() % (*blocks);
		*x = (empty_ID[randomID]).x;
		*y = (empty_ID[randomID]).y;
		free(empty_ID);
	}
}

void RandomPositionConnect(int *x1, int *x2, int *y1, int *y2){
	int k, i, j, randomID, randomID2;
	// Current number of empty blocks
	(*blocks) = (board_x * board_y) - (*bricks) - ((*players) * 2) - (*fruits);
	int threshold = board_x * board_y * 0.3; // Threshold percentage
	if ((*blocks) > threshold)			 // While there are many empty blocks
	{
		// Pacman coordinates
		*x1 = random() % board_x;
		*y1 = random() % board_y;
		while ((*main_board)[*x1][*y1] != EMPTY) // Guarantee that it only finds an empty block
		{
			*x1 = random() % board_x;
			*y1 = random() % board_y;
		}
		// Monster coordinates
		*x2 = random() % board_x;
		*y2 = random() % board_y;
		// Guarantee that it only finds an empty block and that the monster
		// has different coordinates from the Pacman
		while ((*main_board)[*x2][*y2] != EMPTY || ((*x1 == *x2) && (*y1 == *y2)))
		{
			*x2 = random() % board_x;
			*y2 = random() % board_y;
		}
	}
	else // If there aren't many empty blocks
	{
		struct position *empty_ID = malloc((*blocks) * sizeof(position));
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
				if ((*main_board)[i][j] == EMPTY && k < (*blocks))
				{
					(empty_ID[k]).x = i;
					(empty_ID[k]).y = j;
					k++;
				}
			}
		}
		// Pacman coordinates
		randomID = (int)random() % (*blocks);
		*x1 = (empty_ID[randomID]).x;
		*y1 = (empty_ID[randomID]).y;
		// Monster coordinates
		randomID2 = random() % (*blocks);
		while (randomID2 == randomID) // Guarantee coordinates are different
			randomID2 = random() % (*blocks);

		*x2 = (empty_ID[randomID2]).x;
		*y2 = (empty_ID[randomID2]).y;
		(*main_board)[*x1][*y1] = PACMAN;
		(*main_board)[*x2][*y2] = MONSTER;
		free(empty_ID);
	}
}

void IncrementMovement(struct player *my_player){ 
	if(my_player->pacman_tokens < 2)
		my_player->pacman_tokens = my_player->pacman_tokens + 1;
	if(my_player->monster_tokens < 2)
		my_player->monster_tokens = my_player->monster_tokens + 1;
	return;
}

int** loadBoard(char *arg, int *x, int *y){
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int i;

	board_x = *x;
	board_y = *y;

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

if(board_x*board_y < 8)   
{
	printf("Please give a bigger board (with at least 8 empty blocks).\n");
		fclose(fp);
		exit(EXIT_FAILURE);
    }

	// Create contiguous 2D array to store bricks/board in general
	int *temp = calloc(board_x * board_y, sizeof(int));
	int **board = calloc(board_x, sizeof(int *));
	board[0] = temp;
	for (i = 1; i < board_x; i++)
		board[i] = board[i - 1] + board_y;

	//creates a windows and a board
	create_board_window(board_y, board_x);

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
			fclose(fp);
			if (line)
				free(line);
			freeBoard();
			exit(EXIT_FAILURE);
		}
		// For a given line, iterate all the columns
		for (int j = 0; j < board_y; j++)
		{
			// If there is a brick, store in board
			if (line[j] == 'B')
			{
				board[j][i] = 1;
				paint_brick(j, i);
				AddPosHead(j, i, BRICK);
				(*bricks)++;
			}
		}
		// i is the index of current line
		i++;
	}

	// If file doesnt have board_x lines
	if (i != board_x)
	{
		printf("Please give a valid input file.\n");
		fclose(fp);
		if (line)
			free(line);
		freeBoard();
		exit(EXIT_FAILURE);
	}

	if( (board_x*board_y) - (*bricks) < 8){
		printf("Please give a board with at least 8 empty blocks.\n");
		fclose(fp);
		if (line)
			free(line);
		freeBoard();
		exit(EXIT_FAILURE);
	}

	// Close file, free line reader
	fclose(fp);
	if (line)
		free(line);
	
	return board;
}

void ManageFruits(){
	if ((*players) <= 1 && (*fruits) == 0)
		return;
	if ((*fruits) == (*players - 1) * 2)
		return;
	if ((*fruits) > (*players - 1) * 2){
		do{
			int x, y;
			FetchFruitHeadCoords(&x, &y);
			RemoveFruitHead();

			clear_place(x, y);
			(*main_board)[x][y] = 0;
			(*fruits)--;
			broadcast_update(x, y, x, y, (*main_board)[x][y], NULL);

		} while (*fruits > (*players - 1) * 2);

		return;
	}
	do{
		int x, y;
		RandomPositionRules(&x, &y);

		int fruit = rand() % (CHERRY + 1 - LEMON) + LEMON;
		AddPosHead(x, y, fruit);

		(*fruits)++;
		if (fruit == LEMON)	{
			(*main_board)[x][y] = LEMON;
			paint_lemon(x, y);
		}
		else if (fruit == CHERRY)	{
			(*main_board)[x][y] = CHERRY;
			paint_cherry(x, y);
		}
		broadcast_update(x, y, x, y, (*main_board)[x][y], NULL);

	} while ((*fruits) < (*players - 1) * 2);
}