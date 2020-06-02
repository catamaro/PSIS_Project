#ifndef STRUCTS_H
#define STRUCTS_H

#include <pthread.h>

typedef struct pos_update{
  int character; // 2 pacman 3 monster
  int x;
  int y;
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