#ifndef SERVER_H
#define SERVER_H

#include "UI_library/UI_library.h"
#include <pthread.h>
#include <sys/types.h>          
#include <sys/socket.h>

typedef struct exe4_message{
  int character; // 2 pacman 3 monster
  int x;
  int y;
} exe4_message;


#endif