#ifndef STRUCTS_H
#define STRUCTS_H

#include <pthread.h>

// when something moves update position
typedef struct update_msg
{
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
typedef struct init_msg_2
{
  int character;
  int x;
  int y;
  int r;
  int g;
  int b;
} init_msg_2;

// rgb code for pacman and monster
typedef struct color
{
  int r;
  int g;
  int b;
} color;

// struct with player info
typedef struct player
{
  int id;
  int sock_fd;
  pthread_t thread_id;
  struct color *rgb;
  int times;
  int score;
  
  unsigned int inactive_time_pacman;
  unsigned int inactive_time_monster;

  int pacman_tokens;
  int monster_tokens;

  struct position *pacman;
  struct position *monster;

  struct player *next;
} player;

// struct with fruits and bricks info
typedef struct pos_list
{
  int x;
  int y;
  int character;
  struct pos_list *next;
} pos_list;

// struct for positions
struct position
{
  int x;
  int y;
} position;

#endif
