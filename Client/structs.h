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

typedef struct update_msg{
  int character; 
  int x;
  int y;
  int new_x;
  int new_y;
  int r;
  int g;
  int b;
} update_msg;

// message when new player connects for fruits and bricks
typedef struct init_msg_1
{
  int character;
  int x;
  int y;
} init_msg_1;

// message when new player connects for pacman and monster
// message when new player connects for pacman and monster
typedef struct init_msg_2
{
  int character;
  int x;
  int y;
  int r;
  int g;
  int b;
} init_msg_2;

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
  struct color *rgb;

  struct position * pacman;
  struct position * monster;
}player;

#endif
