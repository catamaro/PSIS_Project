#include "structs.h"
#include "server.h"

struct player *head = NULL;

player *getList()
{
   return head;
}

//display the list
void printList()
{
   struct player *ptr = head;
   printf("\n[ ");

   //start from the beginning
   while (ptr != NULL)
   {
      printf("(%d,%d,%d,%d,%d) ", ptr->id, ptr->monster->x, ptr->monster->y,
             ptr->pacman->x, ptr->pacman->y);
      ptr = ptr->next;
   }

   printf(" ]");
}

//delete first item
struct player *deleteFirst()
{

   //save reference to first link
   struct player *tempLink = head;

   //mark next to first link as first
   head = head->next;
   free(tempLink);

   //return the deleted link
   return tempLink;
}

//is list empty
bool isEmpty()
{
   return head == NULL;
}

int length()
{
   int length = 0;
   struct player *current;

   for (current = head; current != NULL; current = current->next)
   {
      length++;
   }

   return length;
}

//find a link with given player
struct player *findPlayer(int player)
{

   //start from the first link
   struct player *current = head;

   //if list is empty
   if (head == NULL)
   {
      return NULL;
   }

   //navigate through list
   while (current->id != player)
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
   struct player *current = head;
   struct player *previous = NULL;

   //if list is empty
   if (head == NULL)
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
   if (current == head)
   {
      free(current);
      //change first to point to next link
      head = head->next;
   }
   else
   {
      //bypass the current link
      previous->next = current->next;
      free(current);
   }
}

// function to insert a player at required position
player * insertPlayer(struct position *pos1, struct position *pos2, int player_id, int fd)
{
   if (head == NULL)
   {
      struct player *link = (struct player *)malloc(sizeof(struct player));
      link->pacman = (struct position *)malloc(sizeof(struct position));
      link->monster = (struct position *)malloc(sizeof(struct position));

      link->pacman->x = pos1->x;
      link->pacman->y = pos1->y;
      link->monster->x = pos2->x;
      link->monster->y = pos2->y;
      link->id = player_id;
      link->sock_fd = fd;

      //point it to old first player
      link->next = head;

      //point first to new first player
      head = link;
      return link;
   }
   //start from the first player
   struct player *current = head;

   //navigate through list
   while (current->id != (player_id - 1))
   {
      //if it is last player
      if (current->next == NULL)
      {
         //create a link
         struct player *link = (struct player *)malloc(sizeof(struct player));
         link->pacman = (struct position *)malloc(sizeof(struct position));
         link->monster = (struct position *)malloc(sizeof(struct position));
         link->pacman->x = pos1->x;
         link->pacman->y = pos1->y;
         link->monster->x = pos2->x;
         link->monster->y = pos2->y;
         link->id = player_id;
         link->sock_fd = fd;
         link->next = NULL;

         return link;
      }
      else
      {
         //move to next link
         current = current->next;
      }
   }
   struct player *link = (struct player *)malloc(sizeof(struct player));
   link->pacman = (struct position *)malloc(sizeof(struct position));
   link->monster = (struct position *)malloc(sizeof(struct position));
   link->pacman->x = pos1->x;
   link->pacman->y = pos1->y;
   link->monster->x = pos2->x;
   link->monster->y = pos2->y;
   link->id = player_id;
   link->sock_fd = fd;

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

void freeList()
{
   struct player *tmp;

   while (head != NULL)
   {
      tmp = head;
      head = head->next;
      close(tmp->sock_fd);
      free(tmp->pacman);
      free(tmp->monster);
      free(tmp);
   }
}