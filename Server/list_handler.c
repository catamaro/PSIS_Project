#include "structs.h"
#include "server.h"

struct node *head = NULL;

//display the list
void printList() {
   struct node *ptr = head;
   printf("\n[ ");

   //start from the beginning
   while(ptr != NULL) {
      printf("(%d,%d,%d,%d) ",ptr->x,ptr->y,ptr->player,ptr->character);
      ptr = ptr->next;
   }

   printf(" ]");
}


//delete first item
struct node* deleteFirst() {

   //save reference to first link
   struct node *tempLink = head;

   //mark next to first link as first
   head = head->next;
   free(tempLink);

   //return the deleted link
   return tempLink;
}

//is list empty
bool isEmpty() {
   return head == NULL;
}

int length() {
   int length = 0;
   struct node *current;

   for(current = head; current != NULL; current = current->next) {
      length++;
   }

   return length;
}

//find a link with given player
struct node* find(int player) {

   //start from the first link
   struct node* current = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->player != player) {

      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }

   //if data found, return the current Link
   return current;
}

//delete a link with given player
void delete(int player) {

   //start from the first link
   struct node* current = head;
   struct node* previous = NULL;
   struct node* temp = NULL;

   //if list is empty
   if(head == NULL) {
      return;
   }

   //navigate through list
   while(current->player != player) {

      //if it is last node
      if(current->next == NULL) {
         return;
      } else {
         //store reference to current link
         previous = current;
         //move to next link
         current = current->next;
      }
   }

   //found a match, update the link
   if(current == head) {
      free(current);
      //change first to point to next link
      head = head->next;
   } else {
      //bypass the current link
      previous->next = current->next;
      free(current);
   }
}

// function to insert a Node at required position
void insertPos(int x, int y, int player, int character)
{
    if(head == NULL)
    {
      struct node *link = (struct node*) malloc(sizeof(struct node));

      link->x = x;
      link->y = y;
      link->player = player;
      link->character = character;

      //point it to old first node
      link->next = head;

      //point first to new first node
      head = link;
      return;
    }
    //start from the first node
    struct node* current = head;

    //navigate through list
    while(current->player != (player-1)) {
       //if it is last node
       if(current->next == NULL) {
         //create a link
         struct node *link = (struct node*) malloc(sizeof(struct node));
         link->x = x;
         link->y = y;
         link->player = player;
         link->character = character;
         current->next = link;
         link->next = NULL;
         return;
       } else {
          //move to next link
          current = current->next;
       }
    }
    struct node *link = (struct node*) malloc(sizeof(struct node));
    link->x = x;
    link->y = y;
    link->player = player;
    link->character = character;
    if(current->next == NULL)
    {
      current->next = link;
      link->next = NULL;
    }
    else
    {
      link->next = current->next;
      current->next = link;
    }

}

void freeList()
{
   struct node* tmp;

   while (head != NULL)
    {
       tmp = head;
       head = head->next;
       free(tmp);
    }

}

void main() {
   insertPos(1,10,1,0);
   insertPos(4,1,4,0);
   insertPos(2,20,2,1);
   insertPos(6,56,6,1);
   insertPos(5,40,5,0);
   insertPos(3,30,3,1);


   printf("Original List: ");

   //print list
   printList();

   while(!isEmpty()) {
      deleteFirst();
   }

   printf("\nList after deleting all items: ");
   printList();
   insertPos(1,10,1,0);
   insertPos(2,20,2,1);
   insertPos(3,30,3,1);
   insertPos(4,1,4,0);
   insertPos(5,40,5,0);
   insertPos(6,56,6,1);

   printf("\nRestored List: ");
   printList();
   printf("\n");

   struct node *foundLink = find(4);

   if(foundLink != NULL) {
      printf("Element found: ");
      printf("(%d,%d,%d,%d) ",foundLink->x,foundLink->y,
                        foundLink->player, foundLink->character);
      printf("\n");
   } else {
      printf("Element not found.");
   }

   delete(4);
   printf("List after deleting an item: ");
   printList();
   printf("\n");
   foundLink = find(4);

   if(foundLink != NULL) {
      printf("Element found: ");
      printf("(%d,%d,%d,%d) ",foundLink->x,foundLink->y,
                        foundLink->player, foundLink->character);
      printf("\n");
   } else {
      printf("Element not found.");
   }

   printf("\n");
   freeList();
}
