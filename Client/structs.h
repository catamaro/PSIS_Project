#ifndef STRUCTS_H
#define STRUCTS_H

#include <pthread.h>

typedef struct update_msg{
  int character; 
  int new_x;
  int new_y;
  int r;
  int g;
  int b;
  int x;
  int y;
} update_msg;

// message when new player connects for fruits and bricks
typedef struct init_msg_1
{
  int character;
  int new_x;
  int new_y;
} init_msg_1;

// message when new player connects for pacman and monster
// message when new player connects for pacman and monster
typedef struct init_msg_2
{
  int character;
  int new_x;
  int new_y;
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
