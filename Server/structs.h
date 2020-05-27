#ifndef STRUCTS_H
#define STRUCTS_H

#include "UI_library.h"
#include <pthread.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct exe4_message{
  int character; // 2 pacman 3 monster
  int x;
  int y;
} exe4_message;

typedef struct player{
  int id;
  int sock_fd;
  struct _player * prev_player;
  struct _player * next_player;

  pthread_t thread_id;

  int x;
  int y;

}player;

struct node {
   int x;
   int y;
   int player;
   int character;
   struct node *next;
};


#endif