#ifndef LIST_HANDLER_H
#define LIST_HANDLER_H

void printList();

void freeList();

struct player * findPlayer(int player);

struct player *findPlayerPos(int x, int y, int type);

void deletePlayer(int player_id);

player * insertPlayer(struct position *pos1, struct position *pos2, 
                        struct color *p_color, int player_id, int fd);
bool isEmpty();

int length();

player * getPlayerList();

fruits *getFruitList();

void AddFruitHead(int x, int y);

void FreeFruitList();

void RemoveFruitPosition(int x, int y);

void RemoveFruitHead();

void FetchFruitHeadCoords();


#endif