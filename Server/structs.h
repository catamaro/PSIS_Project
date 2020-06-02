#ifndef STRUCTS_H
#define STRUCTS_H

#include <pthread.h>

typedef struct exe4_message{
  int character; // 2 pacman 3 monster
  int x;
  int y;
} exe4_message;

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
  pthread_t thread_id;
  struct color *p_color;


  struct position * pacman;
  struct position * monster;

  struct player * next;
}player;


#endif