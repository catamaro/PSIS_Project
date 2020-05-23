#ifndef MAIN_H
#define MAIN_H

#include "UI_library/UI_library.h"
#include <sys/types.h>          
#include <sys/socket.h>
#include <pthread.h>

typedef struct exe4_message{
  int character; // 2 pacman 3 monster
  int x;
  int y;
} exe4_message;

#endif