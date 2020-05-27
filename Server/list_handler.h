#ifndef LIST_HANDLER_H
#define LIST_HANDLER_H


void printList();

void freeList();

struct player * findPlayer(int player);

void deletePlayer(int player_id);

player * insertPlayer(struct position *pos1, struct position *pos2, int player_id, int fd);

bool isEmpty();

int length();

player * getList();


#endif