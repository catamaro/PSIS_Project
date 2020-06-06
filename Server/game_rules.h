#ifndef GAME_RULES_H
#define GAME_RULES_H

void bounceBounds(int x1, int y1, int *x_new, int *y_new);

void RandomPositionConnect(int *x1, int *x2, int *y1, int *y2);

void RandomPositionRules(int *x, int *y);

void checkRulesMonster(struct player *dealer, struct player *receiver, int x_new, int y_new,
					   int *x_new1, int *y_new1, int *x_new2, int *y_new2,
					   int *x_old1, int *y_old1, int *x_old2, int *y_old2);

void checkRulesPacman(struct player *dealer, struct player *receiver, int x_new, int y_new,
					  int *x_new1, int *y_new1, int *x_new2, int *y_new2,
					  int *x_old1, int *y_old1, int *x_old2, int *y_old2);

void checkRulesSuperPacman(struct player *dealer, struct player *receiver, int x_new, int y_new,
					  int *x_new1, int *y_new1, int *x_new2, int *y_new2,
					  int *x_old1, int *y_old1, int *x_old2, int *y_old2);

void IncrementMovement(struct player *my_player);

void ManageFruits();

int ** loadBoard(char *arg, int *x, int *y);

void getValues(unsigned int *update_time, int *bricks, int *players, int *fruits, 
			int *empty, int ***board);


#endif