#ifndef STRUCTS_H
#define STRUCTS_H

#include <pthread.h>

typedef struct pos_update
{
  int character;
  int x;
  int y;
  int new_x;
  int new_y;
} pos_update;

typedef struct color
{
  int r;
  int g;
  int b;
} color;

struct position
{
  int x;
  int y;
  int character;
} position;

typedef struct player
{
  int id;
  int sock_fd;
  pthread_t thread_id;
  struct color *p_color;
  int times;
  int score;

  struct position *pacman;
  struct position *monster;

  struct player *next;
} player;

typedef struct pos_list
{
  int x;
  int y;
  struct pos_list *next;
} pos_list;

#endif
