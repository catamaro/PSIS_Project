#ifndef STRUCTS_H
#define STRUCTS_H

#include <pthread.h>

#define EMPTY 0
#define BRICK 1
#define PACMAN 2
#define MONSTER 3
#define LEMON 4
#define CHERRY 5
#define SUPERPACMAN 6

typedef struct pos_update{
  int character; 
  int x;
  int y;
  int new_x;
  int new_y;
} pos_update;

typedef struct color{
  int r; 
  int g;
  int b;
} color;

struct position {
   int x;
   int y;
}position;

typedef struct player{
  int id;
  int sock_fd;
  struct color *p_color;

  struct position * pacman;
  struct position * monster;
}player;


#endif