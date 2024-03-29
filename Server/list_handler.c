#include "structs.h"
#include "server.h"



struct player *playerHead = NULL;
struct pos_list *fruitHead = NULL;
struct pos_list *brickHead = NULL;

player *getPlayerList(){
   return playerHead;
}

pos_list *getFruitList(){
   return fruitHead;
}

pos_list *getBrickList(){
   return brickHead;
}

//display the list
void printList()
{
   struct player *ptr = playerHead;
   printf("\n[ ");

   //start from the beginning
   while (ptr != NULL)
   {
      printf("(id:%d,x:%d,y:%d,x:%d,y:%d,r:%d,g:%d,b:%d) ", ptr->id, ptr->monster->x,
               ptr->monster->y, ptr->pacman->x, ptr->pacman->y,ptr->rgb->r,
               ptr->rgb->g, ptr->rgb->b);
      ptr = ptr->next;
   }

   printf(" ]");
}
//delete first item
struct player *deleteFirst()
{

   //save reference to first link
   struct player *tempLink = playerHead;

   //mark next to first link as first
   playerHead = playerHead->next;
   free(tempLink);

   //return the deleted link
   return tempLink;
}
//is list empty
bool isEmpty()
{
   return playerHead == NULL;
}

int length(int type)
{
   int length = 0;
   struct player *currentPlayer = playerHead;
   struct pos_list *current;
   struct pos_list *head;

   if(type == CHERRY || type == LEMON) head = fruitHead;
   else if(type == BRICK) head= brickHead;
   else{

      for (currentPlayer = playerHead; currentPlayer != NULL; currentPlayer = currentPlayer->next)
      {
         length++;
      }
      return length;
   } 

   for (current = head; current != NULL; current = current->next)
   {
      length++;
   }

   return length;
}
//find a link with given player
struct player *findPlayer(int id)
{
   //start from the first link
   struct player *current = playerHead;

   //if list is empty
   if (playerHead == NULL)
   {
      return NULL;
   }

   //navigate through list
   while (current->id != id)
   {

      //if it is last player
      if (current->next == NULL)
      {
         return NULL;
      }
      else
      {
         //go to next link
         current = current->next;
      }
   }

   //if data found, return the current Link
   return current;
}
//delete a link with given player
void deletePlayer(int player_id)
{

   //start from the first link
   struct player *current = playerHead;
   struct player *previous = NULL;

   //if list is empty
   if (playerHead == NULL)
   {
      return;
   }

   //navigate through list
   while (current->id != player_id)
   {

      //if it is last player
      if (current->next == NULL)
      {
         return;
      }
      else
      {
         //store reference to current link
         previous = current;
         //move to next link
         current = current->next;
      }
   }

   //found a match, update the link
   if (current == playerHead)
   {
      free(current);
      //change first to point to next link
      playerHead = playerHead->next;
   }
   else
   {
      //bypass the current link
      previous->next = current->next;
      free(current);
   }
}
// function to insert a player at required position
player * insertPlayer(struct position *pos1, struct position *pos2, struct color *rgb, int player_id, int fd)
{
   if (playerHead == NULL)
   {
      struct player *link = (struct player *)malloc(sizeof(struct player));

      link->pacman = pos1;
      link->monster = pos2;
      link->rgb = rgb;
      link->id = player_id;
      link->sock_fd = fd;
      link->times = 0;
      link->next = NULL;
      link->pacman_tokens = 2;
      link->monster_tokens = 2;
      link->inactive_time_pacman = 0;
      link->inactive_time_monster = 0;
      link->score = 0;

      //point it to old first player
      link->next = playerHead;

      //point first to new first player
      playerHead = link;


      return link;
   }
   //start from the first player
   struct player *current = playerHead;

   //navigate through list
   while (current->id != (player_id - 1))
   {
      //if it is last player
      if (current->next == NULL)
      {
         //create a link
         struct player *link = (struct player *)malloc(sizeof(struct player));

         link->pacman = pos1;
         link->monster = pos2;
         link->rgb = rgb;
         link->id = player_id;
         link->sock_fd = fd;
         link->times = 0;
         link->next = NULL;
         link->pacman_tokens = 2;
         link->monster_tokens = 2;
         link->inactive_time_pacman = 0;
         link->inactive_time_monster = 0;
         link->score = 0;

         return link;
      }
      else
      {
         //move to next link
         current = current->next;
      }
   }
   struct player *link = (struct player *)malloc(sizeof(struct player));

   link->pacman = pos1;
   link->monster = pos2;
   link->rgb = rgb;
   link->id = player_id;
   link->sock_fd = fd;
   link->times = 0;
   link->pacman_tokens = 2;
   link->monster_tokens = 2;
   link->inactive_time_pacman = 0;
   link->inactive_time_monster = 0;
   link->score = 0;

   if (current->next == NULL)
   {
      current->next = link;
      link->next = NULL;
   }
   else
   {
      link->next = current->next;
      current->next = link;
   }

   return link;
}
// free player list
void freeList()
{
   struct player *tmp;

   while (playerHead != NULL)
   {
      tmp = playerHead;
      playerHead = playerHead->next;
      close(tmp->sock_fd);
      free(tmp->pacman);
      free(tmp->monster);
      free(tmp->rgb);
      pthread_cancel(tmp->thread_id);

      free(tmp);
   }
}

struct player *findPlayerPos(int x, int y, int type){
//start from the first link
   struct player *current = playerHead;

   //if list is empty
   if (playerHead == NULL)
   {
      printf("error: empty list\n");
      return NULL;
   }

   //navigate through list
   while (current != NULL)
   {
      //if it is last player
      if (current->pacman->x == x && current->pacman->y == y && type == PACMAN)
      {
         return current;
      }
      if (current->monster->x == x && current->monster->y == y && type == MONSTER)
      {
         return current;
      }
      //go to next link
      current = current->next;

   }

   //if data found, return NULL
   return NULL;
}

void AddPosHead(int x, int y, int type){
    struct pos_list* node;
    node = (struct pos_list*)malloc(sizeof(struct pos_list));
    node->x = x;
    node->y = y;
    if(type == BRICK){
        node->next = brickHead;
        node->character = BRICK;
        brickHead = node;
    }
    else if(type == CHERRY){
        node->next = fruitHead;
        node->character = CHERRY;
        fruitHead = node;
    }
    else if(type == LEMON){
        node->next = fruitHead;
        node->character = LEMON;
        fruitHead = node;
    }
}

void freePosList(){
    struct pos_list *tmp;

    while(fruitHead != NULL)
    {
      tmp = fruitHead;
      fruitHead = fruitHead->next;
      free(tmp);
    }

    while(brickHead != NULL)
    {
      tmp = brickHead;
      brickHead = brickHead->next;
      free(tmp);
    }
}

void RemoveFruitPosition(int x, int y){
  //start from the first link
  struct pos_list *current = fruitHead;
  struct pos_list *previous = NULL;

  //if list is empty
  if (fruitHead == NULL)
  {
     return;
  }

  //navigate through list
  while (current->x != x && current->y != y)
  {
     //if it is the last fruit
     if (current->next == NULL)
     {
        return;
     }
     else
     {
        //store reference to current link
        previous = current;
        //move to next link
        current = current->next;
     }
  }

  //found a match, update the link
  if (current == fruitHead)
  {
     free(current);
     //change first to point to next link
     fruitHead = fruitHead->next;
  }
  else
  {
     //bypass the current link
     previous->next = current->next;
     free(current);
  }
}

void RemoveFruitHead(){
    struct pos_list *tempLink = fruitHead;
    if(fruitHead->next != NULL)
    {
      fruitHead = fruitHead->next;
      free(tempLink);
    }
    else
    {
      free(tempLink);
      fruitHead = NULL;
    }
}

void FetchFruitHeadCoords(int* x, int* y){
    *x = fruitHead->x;
    *y = fruitHead->y;
}
